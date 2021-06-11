#include "GSMSerialHandler.h"

GSMSerialHandler::GSMSerialHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial):SerialCharResponseHandler(GSM_SERIAL_BUFFER_SIZE, RESPONSE_SEPARATOR, serial)
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
		HandleCallCMD((CallCMDType)data);
	} else if (timerId == flowTimer) {
		flowTimer = 0;
		OnFlowTimer();
	} else {
		SerialCharResponseHandler::OnTimerComplete(timerId, data);
	}
}


void GSMSerialHandler::HandleCallCMD(CallCMDType type)
{
    switch (type)
    {
    case CallCMDType::DIAL:
        DialCMD();
        break;
    case CallCMDType::HANGUP:
        HangupCallCMD(); // Hangup during call delay
        break;
    case CallCMDType::ANSWER:
        AnswerCallCMD(); // Incoming answer call delay
        break;
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

    if (debugPrint != NULL) {
        if (reqCmd != NULL) {
            debugPrint->print(reqCmd);
            debugPrint->print(" ");
        }
        debugPrint->print("->[");
        debugPrint->write((uint8_t *)buffer, size);
        debugPrint->print("] ");
        debugPrint->println((int)size, 10);
    }

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
                callTimer = Timer::Start(this, CALL_ANSWER_DELAY, CallCMDType::DIAL);
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
		size_t len = SplitString(clccContent, ',', clccArgs, 8, false);
		// remove quotations
		ShiftQuotations(clccArgs, len);

        bool isIncoming = atoi(clccArgs[1]) == 1;
		uint8_t callStateIndex = atoi(clccArgs[2]);
		callState = (GSMCallState)callStateIndex;

        // callType
        // 0 - Outgoing
        // 1 - Incoming

        UpdateCallState(isIncoming, callStateIndex, clccArgs[5], clccArgs[7]);
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


void GSMSerialHandler::HandleDtfm(char code)
{
    if (dtmfCallback == NULL || dtmfCallback(code)) {
        HangupCallCMD();
    }
}


void GSMSerialHandler::UpdateCallState(bool isIncoming, uint8_t callStateIndex, char * phone, char * callerName)
{
    if (debugPrint != NULL) {
        debugPrint->print("UpdateCallState: ");
        debugPrint->println((int)callStateIndex, 10);
    }

    // Outgoing: 2->3->0->6
    // 0 = accepted ongoing call
    // Incoming:
    // 4->0->6

    callState = (GSMCallState)callStateIndex;

    switch (callState)
    {
        case GSMCallState::DIALING: /* Dialing (MO call)  */
            break;
        case GSMCallState::ALERTING:
            StopCallTimer();
            callTimer = Timer::Start(this, CALL_HANGUP_DELAY, CallCMDType::HANGUP);
            break;

        case GSMCallState::INCOMING: /* Incoming (MT call)  */
        {
            StopCallTimer();
            if (IsAuthorized(phone, callerName)) {
                // Wait 0.5 sec then pick up call
                callTimer = Timer::Start(this, CALL_ANSWER_DELAY, CallCMDType::ANSWER);
            } else {
                callTimer = Timer::Start(this, CALL_ANSWER_DELAY, CallCMDType::HANGUP);
            }
            break;
        }
        case GSMCallState::WAITING:
            //StopCallTimer();
            break;
        case GSMCallState::DISCONNECT:
            /* Disconnect */
            //StopCallTimer();
            //callTimer = Timer::Start(this, CALL_ANSWER_DELAY, CallCMDType::DIAL);
            break;
        case GSMCallState::ACTIVE:
            if (!isIncoming) {
                HangupCallCMD();
            }
            break;
        default:
            break;
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
}

void GSMSerialHandler::HandleErrorResponse(char * reqCmd, char *response, size_t size)
{

}

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
	// TODO: If busy, add to stack and send when ready

	if (messageCallback == NULL) return;
	if (IsBusy() || phone == NULL) return;

	this->messageCallback = messageCallback;
    WriteGsmSerial(GSM_SMS_SEND_CMD, false, true, phone, true);

	// Wait "> " response
}

void GSMSerialHandler::DialCMD()
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

	DialCMD();
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
