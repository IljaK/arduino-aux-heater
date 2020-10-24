#include "GSMSerialHandler.h"
#include "serial/DebugSerialHandler.h"

GSMSerialHandler::GSMSerialHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial):SerialCharResponseHandler(RESPONSE_SEPARATOR, serial)//SerialTimerResponseHandler(stream)
{
	this->smsCallback = smsCallback;
	this->dtmfCallback = dtmfCallback;
	smsSender[0] = 0;
	//primaryPhone[0] = 0;

	flowState = GSMFlowState::INITIALIZATION;
	flowTimer = Timer::Start(this, SERIAL_RESPONSE_TIMEOUT);
}

GSMSerialHandler::~GSMSerialHandler()
{
	if (flowTimer != 0) Timer::Stop(flowTimer);
	flowTimer = 0;
	StopCallTimer();
	if (callDelayTimer != 0) Timer::Stop(callDelayTimer);
	callDelayTimer = 0;
}
void GSMSerialHandler::OnTimerComplete(TimerID timerId)
{
	if (callDelayTimer == timerId) {
		callDelayTimer = 0;
		CallCMD();
	} else if (callTimer == timerId) {
		callTimer = 0;
		switch (callState)
		{
		case GSMCallState::ALERTING:
			HangupCallCMD(); // Hangup during call delay
			break;
		case GSMCallState::INCOMING:
			AnswerCallCMD(); // Incoming answer call delay
			break;
		}
	} else if (timerId == flowTimer) {
		flowTimer = 0;
		SendFlowCMD();
	} else {
		SerialCharResponseHandler::OnTimerComplete(timerId);
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

    if (DebugSerialHandler::IsDebugEnabled()) {
        DebugSerialHandler::outWrite("->[");
        DebugSerialHandler::outWrite(buffer);
        DebugSerialHandler::outWrite("] [");
        DebugSerialHandler::outWriteASCII(size);
        DebugSerialHandler::outWrite("], type: [");
        DebugSerialHandler::outWriteASCII((uint8_t)flowState);
        DebugSerialHandler::outWrite("]\r\n");
    }

	if (isTimeOut) {
		HandleErrorResponse(buffer, size);
		return;
	}

	switch (smsState)
	{
		case GSMIncomingMessageState::NONE:
			// Nothing to do here
			break;
		case GSMIncomingMessageState::AUTH_INCOMING_MSG:
			if (!isOverFlow) { // IF Message is very long, wait till end
				smsState = GSMIncomingMessageState::NONE;
				StartCallDelayTimer();
				if (smsCallback) smsCallback(buffer, size, smsDispatchUTCts);
			}
			return;
		case GSMIncomingMessageState::NOT_AUTH_INCOMING_MSG:
			if (!isOverFlow) { // IF Message is very long, wait till end
				smsState = GSMIncomingMessageState::NONE;
				StartCallDelayTimer();
				// Skip command
			}
			return;
	}	

	if (strcmp(buffer, GSM_OK_RESPONSE) == 0) {
		HandleOKResponse(buffer, size);
		return;
	}

	if (strstr(buffer, GSM_ERROR_RESPONSE) != NULL) {
		HandleErrorResponse(buffer, size);
		return;
	}
	switch (flowState)
	{
		case GSMFlowState::SEND_SMS_BEGIN:
			flowState = GSMFlowState::SEND_SMS_FLOW;
			FinalizeSendMessage();
			break;
		default:
			HandleDataResponse(buffer, size);
			break;
	}
}

void GSMSerialHandler::HandleDataResponse(char *response, size_t size)
{
	
	if (strncmp(response, GSM_REG_CMD, strlen(GSM_REG_CMD)) == 0) {
		// +CREG: 1,2
		// Check roaming state
		char *creg = response + strlen(GSM_REG_CMD) + 2;
		char *cregArgs[2];

		SplitString(creg, ',', cregArgs, 2, false);
		cRegState = atoi(cregArgs[1]);
		if (IsNetworkConnected() && flowState == GSMFlowState::READY) {
			flowState = GSMFlowState::TIME_REQUEST;
			SendFlowCMD();
		}
	}
	else if (strncmp(response, GSM_TS_DATA_CMD, strlen(GSM_TS_DATA_CMD)) == 0) {
		// +CMT: "+372000000",,"2019/07/25,23:25:32+03"
		// +CMT: "+32353","2356 aux-1","19/07/31,13:29:43+12"
		char *cmt = response + strlen(GSM_TS_DATA_CMD) + 2;
		char *cmtArgs[3];
		size_t len = SplitString(cmt, ',', cmtArgs, 3, false);

		// remove quotations
		ShiftQuotations(cmtArgs, len);

		char *senderName = cmtArgs[1];
		if (IsAuthorized(senderName)) {
			strcpy(smsSender, cmtArgs[0]);
			tmZone smsDate;
			timeLocalStruct(cmtArgs[2], &smsDate);
			smsDate.tm_isdst = 0;
			smsDispatchUTCts = mk_gmtime(&smsDate) - smsDate.ZoneInSeconds();
			smsState = GSMIncomingMessageState::AUTH_INCOMING_MSG;
		} else {
			smsState = GSMIncomingMessageState::NOT_AUTH_INCOMING_MSG;
		}
	}
	else if (strncmp(response, GSM_CALL_STATE_CMD, strlen(GSM_CALL_STATE_CMD)) == 0) {
		// +CLCC: 1,1,4,0,0,"+37211111",145,"ilja aux-1"

		char *clccContent = response + strlen(GSM_CALL_STATE_CMD) + 2;
		char *clccArgs[8];
		size_t len = SplitString(clccContent, ',', clccArgs, 8, false);

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
				callTimer = Timer::Start(this, CALL_HANGUP_DELAY);
				break;

			case GSMCallState::INCOMING: /* Incoming (MT call)  */
			{
				if (IsAuthorized(clccArgs[7])) {
					// Wait 0.5 sec then pick up call
					StopCallTimer();
					callTimer = Timer::Start(this, CALL_ANSWER_DELAY);
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
				StartCallDelayTimer();
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
	// +CPBF: 1,\"+372111111\",145,\"1 aux-1"
	else if (strncmp(response, GSM_FIND_USER_CMD, strlen(GSM_FIND_USER_CMD)) == 0) 
	{
		char *cpbf = response + strlen(GSM_FIND_USER_CMD) + 1;
		char *cpbfArgs[4];
		SplitString(cpbf, ',', cpbfArgs, 4, false);
		// remove quotations
		ShiftQuotations(cpbfArgs, 4);

		char *userName = cpbfArgs[3];
		char *auxChar = strstr(userName, GSM_AUX_PHONE_POSTFIX);

		if (auxChar == NULL) return;

		auxChar += strlen(GSM_AUX_PHONE_POSTFIX);
		uint8_t index = atoi(auxChar);
		if (lowestIndex >= index)
		{
			lowestIndex = index;
			strcpy(primaryPhone, cpbfArgs[1]);
		}
	}
	
	// +CCLK: "20/08/25,21:08:38+12"
	else if (strncmp(response, GSM_TIME_CMD, strlen(GSM_TIME_CMD)) == 0) 
	{
		char *cclk = response + strlen(GSM_TIME_CMD) + 2;
		tmZone tmStruct;
		timeLocalStruct(cclk, &tmStruct);
		setSystemTime(&tmStruct);
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

void GSMSerialHandler::HandleOKResponse(char *response, size_t size)
{
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
			flowState = GSMFlowState::FIND_PRIMARY_PHONE;
			break;
		default:
			flowState = GSMFlowState::LOCKED;
			return;
		}
		break;
	}
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::FIND_PRIMARY_PHONE;
		break;

	case GSMFlowState::CALL_DIAL:
		callState = GSMCallState::DIALING;
		flowState = GSMFlowState::READY;
		break;
	case GSMFlowState::FIND_PRIMARY_PHONE:
        if (DebugSerialHandler::IsDebugEnabled()) {
            DebugSerialHandler::outWrite(F("Primary phone: "));
            DebugSerialHandler::outWrite(primaryPhone);
			DebugSerialHandler::outWriteEnd();
        }
		//break;
	case GSMFlowState::TIME_REQUEST:
	case GSMFlowState::SEND_SMS_FLOW:
	case GSMFlowState::CALL_HANGUP:
	case GSMFlowState::CALL_ANSWER:
		flowState = GSMFlowState::READY;
		break;
	}
	StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
}

void GSMSerialHandler::HandleErrorResponse(char *response, size_t size)
{
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
}

void GSMSerialHandler::SendFlowCMD(char *data)
{
   switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
        WriteGsmSerial(NULL);
		break;

	case GSMFlowState::TIME_REQUEST:
        WriteGsmSerial((char *)GSM_TIME_CMD, true);
		break;

	case GSMFlowState::FIND_PRIMARY_PHONE:
        WriteGsmSerial((char*)GSM_FIND_USER_CMD, false, true, (char*)GSM_AUX_PHONE_POSTFIX, true);
		break;

	case GSMFlowState::SIM_PIN_STATE:
        WriteGsmSerial((char*)GSM_SIM_PIN_CMD, true);
		break;

	case GSMFlowState::SIM_LOGIN:
        WriteGsmSerial((char*)GSM_SIM_PIN_CMD, false, true, (char*)SIM_PIN_CODE, true);
		break;
	case GSMFlowState::SEND_SMS_BEGIN:
        WriteGsmSerial((char*)GSM_SMS_SEND_CMD, false, true, primaryPhone, true);
		break;
	case GSMFlowState::CALL_DIAL:
        WriteGsmSerial((char*)GSM_CALL_DIAL_CMD, false, true, data, false, true);
        break;
	case GSMFlowState::CALL_HANGUP:
        WriteGsmSerial((char*)GSM_CALL_HANGUP_CMD);
		break;
	case GSMFlowState::CALL_ANSWER:
        WriteGsmSerial((char*)GSM_CALL_ANSWER_CMD);
		break;
	}
}

void GSMSerialHandler::WriteGsmSerial(char * cmd, bool isCheck, bool isSet, char *data, bool dataQuotations, bool semicolon)
{
	if (serial) {
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		
		if (cmd != NULL) {
			serial->write(cmd, strlen(cmd));
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
			serial->write(data, strlen(data));
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
		flowState != GSMFlowState::READY || 
		smsState != GSMIncomingMessageState::NONE || 
		callState != GSMCallState::DISCONNECT ||
		callTimer != 0 ||
		callDelayTimer != 0;
}

void GSMSerialHandler::SendSMSMessage(StreamCallback messageCallback)
{
	// TODO: If busy, add to stack and send on ready

	if (messageCallback == NULL) return;
	if (IsBusy() || primaryPhone[0] == 0) return;

	this->messageCallback = messageCallback;
	flowState = GSMFlowState::SEND_SMS_BEGIN;
	SendFlowCMD();

	// Wait "> " response
}

void GSMSerialHandler::CallCMD()
{
	if (IsBusy()) return;

	char *phone = callHangupStack.UnshiftFirst();

	if (phone == NULL) return;

	flowState = GSMFlowState::CALL_DIAL;
	SendFlowCMD(phone);

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

	flowState = GSMFlowState::CALL_HANGUP;
	SendFlowCMD();
}
void GSMSerialHandler::AnswerCallCMD()
{
	StopCallTimer();

	flowState = GSMFlowState::CALL_ANSWER;
	SendFlowCMD();
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
	if (flowState == GSMFlowState::SEND_SMS_BEGIN && !result) {
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

GSMFlowState GSMSerialHandler::FlowState()
{
	return flowState;
}

bool GSMSerialHandler::IsAuthorized(char *entryName)
{
	return entryName != NULL && entryName[0] != 0 && strstr(entryName, GSM_AUX_PHONE_POSTFIX) != NULL;
}

GSMCallState GSMSerialHandler::CallState()
{
	return callState;
}
GSMIncomingMessageState GSMSerialHandler::SMSState()
{
	return smsState;
}

void GSMSerialHandler::StartCallDelayTimer()
{
	if (callDelayTimer != 0) {
		Timer::Stop(callDelayTimer);
		callDelayTimer = 0;
	}
	callDelayTimer = Timer::Start(this, GSM_CALL_DELAY);
}
