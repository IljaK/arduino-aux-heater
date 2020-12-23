#include "GSMSerialHandler.h"

GSMSerialHandler::GSMSerialHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial):SerialCharResponseHandler(RESPONSE_SEPARATOR, serial)//SerialTimerResponseHandler(stream)
{
	this->smsCallback = smsCallback;
	this->dtmfCallback = dtmfCallback;
	smsSender[0] = 0;
    flowTimer = 0;
}

GSMSerialHandler::~GSMSerialHandler()
{
	if (flowTimer != 0) Timer::Stop(flowTimer);
	flowTimer = 0;
	StopCallTimer();
}
void GSMSerialHandler::OnTimerComplete(TimerID timerId, uint8_t data)
{
	if (callTimer == timerId) {
		callTimer = 0;
		switch (data)
		{
        case CallTimerState::DIAL:
            CallCMD();
            break;
		case CallTimerState::HANGUP:
			HangupCallCMD(); // Hangup during call delay
			break;
		case CallTimerState::ANSWER:
			AnswerCallCMD(); // Incoming answer call delay
			break;
		default:
			break;
		}
	} else if (timerId == flowTimer) {
		flowTimer = 0;
		OnFlowTimer();
	} else {
		SerialCharResponseHandler::OnTimerComplete(timerId, data);
	}
}

void GSMSerialHandler::StartFlowTimer(unsigned long duration)
{
	if (flowTimer != 0) Timer::Stop(flowTimer);
	flowTimer = Timer::Start(this, duration);
}

void GSMSerialHandler::OnResponseReceived(bool isTimeOut, bool isOverFlow)
{
	SerialCharResponseHandler::OnResponseReceived(isTimeOut);
	size_t size = strlen(buffer);

	if (size == 0 && !isTimeOut) return;

    //if (DebugHandler::IsDebugEnabled()) {
        if (reqCmd != NULL) {
            DebugHandler::outWrite(reqCmd);
            DebugHandler::outWrite(" ");
        }
        DebugHandler::outWrite("->[");
        DebugHandler::outWrite(buffer, size);
        DebugHandler::outWrite("] ");
        DebugHandler::outWriteASCII((int)size, 10);
        DebugHandler::outWriteEnd();
   // }

    char *cmd = reqCmd;
    reqCmd = NULL;

	if (isTimeOut) {
		HandleErrorResponse(cmd, buffer, size);
		return;
	}

	switch (smsState)
	{
		case GSMSMSState::RECEIVE_AUTH:
			if (!isOverFlow) { // IF Message is very long, wait till end
				smsState = GSMSMSState::NONE;
                callTimer = Timer::Start(this, CALL_ANSWER_DELAY, CallTimerState::DIAL);
				if (smsCallback) smsCallback(buffer, size, smsDispatchUTCts);
			}
			return;
		case GSMSMSState::RECEIVE_NOT_AUTH:
			if (!isOverFlow) { // IF Message is very long, wait till end
				smsState = GSMSMSState::NONE;
				// Skip command
			}
			return;
        case GSMSMSState::SEND_BEGIN:
            smsState = GSMSMSState::SEND_FLOW;
            FinalizeSendMessage();
            return;
        default:
            break;
	}

	if (strcmp(buffer, GSM_OK_RESPONSE) == 0) {
		HandleOKResponse(cmd, buffer, size);
		return;
	}

	if (strstr(buffer, GSM_ERROR_RESPONSE) != NULL) {
		HandleErrorResponse(cmd, buffer, size);
		return;
	}

    reqCmd = cmd;
    HandleDataResponse(cmd, buffer, size);
}

void GSMSerialHandler::HandleDataResponse(char * reqCmd, char *response, size_t size)
{
	if (strncmp(response, GSM_REG_CMD, strlen(GSM_REG_CMD)) == 0) {
		// +CREG: 1,2
		// Check roaming state
		char *creg = response + strlen(GSM_REG_CMD) + 2;
		char *cregArgs[2];
        cregArgs[1] = 0;

		SplitString(creg, ',', cregArgs, 2, false);
        if (cregArgs[1] == 0) {
            cRegState = atoi(cregArgs[0]);
        } else {
		    cRegState = atoi(cregArgs[1]);
        }
		OnNetworkStateUpdated();
	}
	else if (strncmp(response, GSM_TS_DATA_CMD, strlen(GSM_TS_DATA_CMD)) == 0) {
		// +CMT: "+372000000",,"2019/07/25,23:25:32+03"
		// +CMT: "+32353","2356 aux-1","19/07/31,13:29:43+12"
		char *cmt = response + strlen(GSM_TS_DATA_CMD) + 2;
		char *cmtArgs[3];
		size_t len = SplitString(cmt, ',', cmtArgs, 3, false);

		// remove quotations
		ShiftQuotations(cmtArgs, len);

		//char *senderName = cmtArgs[1];
		if (IsAuthorized(cmtArgs[0], cmtArgs[1])) {
			strcpy(smsSender, cmtArgs[0]);
			tmZone smsDate;
			TimeManager::TimeLocalStruct(cmtArgs[2], &smsDate);
			smsDate.tm_isdst = 0;
			smsDispatchUTCts = TimeManager::GetTimeSeconds(&smsDate) - smsDate.ZoneInSeconds();
			smsState = GSMSMSState::RECEIVE_AUTH;
		} else {
			smsState = GSMSMSState::RECEIVE_NOT_AUTH;
		}
	}
	else if (strncmp(response, GSM_CALL_STATE_CMD, strlen(GSM_CALL_STATE_CMD)) == 0) {
		// +CLCC: 1,1,4,0,0,"+37211111",145,"ilja aux-1"

		char *clccContent = response + strlen(GSM_CALL_STATE_CMD) + 2;
		char *clccArgs[8];
		SplitString(clccContent, ',', clccArgs, 8, false);

		uint8_t callType = atoi(clccArgs[1]);
		uint8_t callStateIndex = atoi(clccArgs[2]);
		callState = (GSMCallState)callStateIndex;

		// callType
		// 0 - Incoming
		// 1 - outgoing

		// Outgoing: 2->3->0->6
		// 0 = accepted ongoing call
		// Incoming:
		// 4->0->6

		switch (callState)
		{
			case GSMCallState::DIALING: /* Dialing (MO call)  */
				break;
			case GSMCallState::ALERTING:
				StopCallTimer();
				callTimer = Timer::Start(this, CALL_HANGUP_DELAY, CallTimerState::HANGUP);
				break;

			case GSMCallState::INCOMING: /* Incoming (MT call)  */
			{
				if (IsAuthorized(clccArgs[5], clccArgs[7])) {
					// Wait 0.5 sec then pick up call
					StopCallTimer();
					callTimer = Timer::Start(this, CALL_ANSWER_DELAY, CallTimerState::ANSWER);
				} else {
					HangupCallCMD();
				}
				break;
			}
			case GSMCallState::WAITING:
				StopCallTimer();
				break;
			case GSMCallState::DISCONNECT:
				/* Disconnect */
				StopCallTimer();
                callTimer = Timer::Start(this, CALL_ANSWER_DELAY, CallTimerState::DIAL);
				break;
			case GSMCallState::ACTIVE:
			{
				if (callType == 0) {
					HangupCallCMD();
				}
				break;
			}
			default:
				break;
		}
	}
	else if (strncmp(response, GSM_DTMF_CMD, strlen(GSM_DTMF_CMD)) == 0) {
		// +DTMF: 1
		char *pCode = response + strlen(GSM_DTMF_CMD) + 2;
		char code = pCode[0];
		if (dtmfCallback == NULL || dtmfCallback(code)) {
			HangupCallCMD();
		}
	}
	else if (strncmp(response, GSM_TIME_CMD, strlen(GSM_TIME_CMD)) == 0) {
	    // +CCLK: "20/08/25,21:08:38+12"
		char *cclk = response + strlen(GSM_TIME_CMD) + 2;
		tmZone tmStruct;
		TimeManager::TimeLocalStruct(cclk, &tmStruct);
		TimeManager::UpdateSystemTime(&tmStruct);
	}
	else if (strncmp(response, GSM_SIM_PIN_CMD, strlen(GSM_SIM_PIN_CMD)) == 0) {
		char *simState = response + strlen(GSM_SIM_PIN_CMD) + 2;

		if (strncmp(GSM_SIM_STATE_READY, simState, strlen(GSM_SIM_STATE_READY)) == 0) {
			simPinState = GSMSimPinState::SIM_PIN_STATE_READY;
		}
		else if (strncmp(GSM_SIM_STATE_SIM_PIN, simState, strlen(GSM_SIM_STATE_SIM_PIN)) == 0) {
			simPinState = GSMSimPinState::SIM_PIN_STATE_PIN;
		}
		else {
			simPinState = GSMSimPinState::SIM_PIN_STATE_ERROR;
		}
	}
}


void GSMSerialHandler::OnCMDRequest(char * reqCmd)
{
    if (strncmp(reqCmd, GSM_CALL_DIAL_CMD, strlen(reqCmd)) == 0) {
        callState = GSMCallState::DIALING;
    } else if (strncmp(reqCmd, GSM_SMS_SEND_CMD, strlen(reqCmd)) == 0) {
        smsState = GSMSMSState::SEND_BEGIN;
    }
}

void GSMSerialHandler::HandleOKResponse(char * reqCmd, char *response, size_t size)
{
    /*
	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
		flowState = GSMFlowState::SIM_PIN_STATE;
		break;
	case GSMFlowState::SIM_PIN_STATE:
	{
		switch (simPinState)
		{
		case GSMSimPinState::SIM_PIN_STATE_PIN:
			flowState = GSMFlowState::SIM_LOGIN;
			break;
		case GSMSimPinState::SIM_PIN_STATE_READY:
			flowState = GSMFlowState::REG_NETWORK;
			break;
		default:
			flowState = GSMFlowState::LOCKED;
			return;
		}
		break;
	}
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::REG_NETWORK;
		break;
	case GSMFlowState::REG_NETWORK:
		flowState = GSMFlowState::FIND_PRIMARY_PHONE;
		break;

	case GSMFlowState::CALL_DIAL:
		callState = GSMCallState::DIALING;
		flowState = GSMFlowState::READY;
		break;
	case GSMFlowState::FIND_PRIMARY_PHONE:
        if (DebugHandler::IsDebugEnabled()) {
            DebugHandler::outWrite(F("Primary phone: "));
            DebugHandler::outWrite(primaryPhone);
			DebugHandler::outWriteEnd();
        }
		//break;
	case GSMFlowState::TIME_REQUEST:
	case GSMFlowState::SEND_SMS_FLOW:
	case GSMFlowState::SEND_SMS_BEGIN:
	case GSMFlowState::CALL_HANGUP:
	case GSMFlowState::CALL_ANSWER:
		flowState = GSMFlowState::READY;
		break;
	default:
		return;
	}
	StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
    */
}

void GSMSerialHandler::HandleErrorResponse(char * reqCmd, char *response, size_t size)
{
    /*
	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
		// Just stay in this state => retry
		break;
	case GSMFlowState::SIM_PIN_STATE:
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::LOCKED;
		return;
	default:
		// TODO: Make few attemts then go to error or ready
		flowState = GSMFlowState::READY;
		break;
	}
	StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
    */
}

/*
void GSMSerialHandler::SendFlowCMD(char *data)
{
   switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
        WriteGsmSerial(NULL);
		break;

	case GSMFlowState::TIME_REQUEST:
        WriteGsmSerial(GSM_TIME_CMD, true);
		break;

	case GSMFlowState::REG_NETWORK:
        WriteGsmSerial(GSM_FIND_USER_CMD, false, true, data, true);
		break;

	case GSMFlowState::FIND_PRIMARY_PHONE:
        WriteGsmSerial(GSM_REG_CMD, false, true, (char*)GSM_AUX_PHONE_POSTFIX, true);
		break;

	case GSMFlowState::SIM_PIN_STATE:
        WriteGsmSerial(GSM_SIM_PIN_CMD, true);
		break;

	case GSMFlowState::SIM_LOGIN:
        WriteGsmSerial(GSM_SIM_PIN_CMD, false, true, (char*)SIM_PIN_CODE, true);
		break;
	case GSMFlowState::SEND_SMS_BEGIN:
        WriteGsmSerial(GSM_SMS_SEND_CMD, false, true, primaryPhone, true);
		break;
	case GSMFlowState::CALL_DIAL:
        WriteGsmSerial(GSM_CALL_DIAL_CMD, false, true, data, false, true);
        break;
	case GSMFlowState::CALL_HANGUP:
        WriteGsmSerial(GSM_CALL_HANGUP_CMD);
		break;
	case GSMFlowState::CALL_ANSWER:
        WriteGsmSerial(GSM_CALL_ANSWER_CMD);
		break;
	default:
		break;
	}
}
*/

void GSMSerialHandler::WriteGsmSerial(const char * cmd, bool isCheck, bool isSet, char *data, bool dataQuotations, bool semicolon)
{
    reqCmd = (char *)cmd;

	if (serial) {
        OnCMDRequest(reqCmd);

		serial->write(GSM_INIT_CMD);
		
		if (cmd != NULL) {
			serial->write(cmd);
		}
		if (isCheck) {
			serial->write('?');
		}
		else if (isSet) {
			serial->write('=');
		}
		if (data != NULL) {
			if (dataQuotations) {
				serial->write('\"');
			}
			serial->write(data);
			if (dataQuotations) {
				serial->write('\"');
			}
		}
		if (semicolon) {
			serial->write(';');
		}

		//FlushData();

		serial->write(CR_ASCII_SYMBOL);
	}
	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT);
}

bool GSMSerialHandler::IsProperResponse(char *response, size_t size) {

	for (size_t i = 0; i < size; i++) {
		if (response[i] <= 31u) {
			if (response[i] != CR_ASCII_SYMBOL || response[i] != LF_ASCII_SYMBOL) {
				return false;
			}
		}
	}
	return true;
}

bool GSMSerialHandler::IsBusy()
{
	return SerialCharResponseHandler::IsBusy() || 
		reqCmd != NULL || 
		smsState != GSMSMSState::NONE || 
		callState != GSMCallState::DISCONNECT ||
		callTimer != 0;
}

void GSMSerialHandler::SendSMSMessage(StreamCallback messageCallback, char * phone)
{
	// TODO: If busy, add to stack and send on ready

	if (messageCallback == NULL) return;
	if (IsBusy() || phone == NULL) return;

	this->messageCallback = messageCallback;
    WriteGsmSerial(GSM_SMS_SEND_CMD, false, true, phone, true);

	// Wait "> " response
}

void GSMSerialHandler::CallCMD()
{
	if (IsBusy()) return;

	char *phone = callHangupStack.UnshiftFirst();

	if (phone == NULL) return;

	//flowState = GSMFlowState::CALL_DIAL;
	//SendFlowCMD(phone);
    WriteGsmSerial(GSM_CALL_DIAL_CMD, false, true, phone, false, true);

	free(phone);
	phone = NULL;
}

void GSMSerialHandler::NotifyByCallHangUp()
{
	if (smsSender[0] == 0) return;
	if (!callHangupStack.AppendCopy(smsSender)) return;

	CallCMD();
}

void GSMSerialHandler::HangupCallCMD()
{
	StopCallTimer();
    WriteGsmSerial(GSM_CALL_HANGUP_CMD);
}
void GSMSerialHandler::AnswerCallCMD()
{
	StopCallTimer();
    WriteGsmSerial(GSM_CALL_ANSWER_CMD);
}

void GSMSerialHandler::StopCallTimer()
{
	if (callTimer != 0) {
		Timer::Stop(callTimer);
		callTimer = 0;
	}
}

void GSMSerialHandler::FinalizeSendMessage()
{
	if (serial)
	{
		if (messageCallback(serial))
		{
			//Send message end
			serial->write(CRTLZ_ASCII_SYMBOL);
		}
		else {
			// Send message cancel callback
			serial->write(ESC_ASCII_SYMBOL);
		}
	}
	messageCallback = NULL;

	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT);
}

bool GSMSerialHandler::LoadSymbolFromBuffer(uint8_t symbol)
{
	bool result = SerialCharResponseHandler::LoadSymbolFromBuffer(symbol);

		// Check sms text input extra response
	if (smsState == GSMSMSState::SEND_BEGIN && !result) {
		if (bufferLength == strlen(GSM_MSG_TEXT_INPUT_RESPONSE)) {
			if (strncmp(buffer, GSM_MSG_TEXT_INPUT_RESPONSE, bufferLength) == 0) {
				ResponseDetectedInternal(false, false);
				return true;
			}
		}
	}
	
	return result;
}

bool GSMSerialHandler::IsNetworkConnected()
{
	return cRegState == 1u || cRegState == 5u;
}
bool GSMSerialHandler::IsRoaming()
{
	return cRegState == 5u;
}

GSMCallState GSMSerialHandler::CallState()
{
	return callState;
}
GSMSMSState GSMSerialHandler::SMSState()
{
	return smsState;
}
GSMSimPinState GSMSerialHandler::SimPinState()
{
    return simPinState;
}

char *GSMSerialHandler::PendingRequest()
{
    return reqCmd;
}
