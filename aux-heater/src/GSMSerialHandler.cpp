#include "GSMSerialHandler.h"

GSMSerialHandler::GSMSerialHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial):SerialCharResponseHandler(GSM_RESPONSE_SEPARATOR, serial)//SerialTimerResponseHandler(stream)
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
		LaunchFlowRequest();
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

	outPrintf("->[%s] [%d], type: [%d]", buffer, size, (uint8_t)flowState);

	if (isTimeOut) {
		HandleErrorResponse(buffer, size);
		return;
	}

	switch (smsState)
	{
		case IncomingMessageState::NONE:
			// Nothing to do here
			break;
		case IncomingMessageState::AUTH_INCOMING_MSG:
			if (!isOverFlow) { // IF Message is very long, wait till end
				smsState = IncomingMessageState::NONE;
				StartCallDelayTimer();
				if (smsCallback) smsCallback(buffer, size, smsSendTS);
			}
			return;
		case IncomingMessageState::NOT_AUTH_INCOMING_MSG:
			if (!isOverFlow) { // IF Message is very long, wait till end
				smsState = IncomingMessageState::NONE;
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
	// +CPIN
	if (strncmp(response, GSM_SIM_PIN_CMD, strlen(GSM_SIM_PIN_CMD)) == 0) {
		char *simState = response + strlen(GSM_SIM_PIN_CMD) + 2;

		if (strncmp(GSM_SIM_STATE_READY, simState, strlen(GSM_SIM_STATE_READY)) == 0) {
			flowState = GSMFlowState::SIM_PIN_STATE_READY;
		}
		else if (strncmp(GSM_SIM_STATE_SIM_PIN, simState, strlen(GSM_SIM_STATE_SIM_PIN)) == 0 && !tryedPass) {
			flowState = GSMFlowState::SIM_PIN_STATE_PIN;
		}
		else {
			flowState = GSMFlowState::SIM_PIN_STATE_UNKNOWN;
		}
	}
	// +CMT: "+372000000",,"2019/07/25,23:25:32+03"
	// +CMT: "+32353","2356 aux-1","19/07/31,13:29:43+12"

	else if (strncmp(response, GSM_TS_DATA_CMD, strlen(GSM_TS_DATA_CMD)) == 0) {
		char *cmt = response + strlen(GSM_TS_DATA_CMD) + 2;
		char *cmtArgs[3];
		size_t len = SplitString(cmt, COMMA_ASCII_SYMBOL, cmtArgs, 3, false);

		// remove quotations
		ShiftQuotations(cmtArgs, len);

		char *senderName = cmtArgs[1];
		if (IsAuthorized(senderName)) {
			strcpy(smsSender, cmtArgs[0]);
			tm smsDate;
			timeStruct(cmtArgs[2], &smsDate);
			smsSendTS = mk_gmtime(&smsDate);
			smsState = IncomingMessageState::AUTH_INCOMING_MSG;
		} else {
			smsState = IncomingMessageState::NOT_AUTH_INCOMING_MSG;
		}
	}
	else if (strncmp(response, GSM_REG_CMD, strlen(GSM_REG_CMD)) == 0) {
		// +CREG: 1,2
		// Check roaming state
		char *creg = response + strlen(GSM_REG_CMD) + 2;
		char *cregArgs[2];

		SplitString(creg, COMMA_ASCII_SYMBOL, cregArgs, 2, false);
		cRegState = atoi(cregArgs[1]);
	}
	else if (strcmp(response, GSM_SIM_AUTH_READY) == 0) {
		if (flowState == GSMFlowState::WAIT_SIM_INIT) {
			flowState = GSMFlowState::TIME_REQUEST;
			LaunchFlowRequest();
		}
	}
	else if (strncmp(response, GSM_CALL_STATE_CMD, strlen(GSM_CALL_STATE_CMD)) == 0) {
		// +CLCC: 1,1,4,0,0,"+37211111",145,"ilja aux-1"

		char *clccContent = response + strlen(GSM_CALL_STATE_CMD) + 2;
		char *clccArgs[8];
		size_t len = SplitString(clccContent, COMMA_ASCII_SYMBOL, clccArgs, 8, false);

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
		char *pCode = response + strlen(GSM_CALL_STATE_CMD) + 2;
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
		SplitString(cpbf, COMMA_ASCII_SYMBOL, cpbfArgs, 4, false);
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
		char *cclk = response + strlen(GSM_FIND_USER_CMD) + 2;
		tm tmStruct;
		timeStruct(cclk, &tmStruct);
		setSystemTime(&tmStruct);
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
		break;
	case GSMFlowState::SIM_PIN_STATE_READY:
		flowState = GSMFlowState::TIME_REQUEST;
		break;
	case GSMFlowState::SIM_PIN_STATE_PIN:
		flowState = GSMFlowState::SIM_LOGIN;
		break;
	case GSMFlowState::SIM_PIN_STATE_UNKNOWN:
		flowState = GSMFlowState::LOCKED;
		break;
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::WAIT_SIM_INIT;
		return; // Wait for connection established
		break;
	case GSMFlowState::TIME_REQUEST:
		flowState = GSMFlowState::FIND_PRIMARY_PHONE;
		break;
	case GSMFlowState::CALL_DIAL:
		callState = GSMCallState::DIALING;
		flowState = GSMFlowState::READY;
		break;
	case GSMFlowState::FIND_PRIMARY_PHONE:
		outPrintf("GSM READY! Primary phone: %s", primaryPhone);
		//break;
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
	case GSMFlowState::SIM_PIN_STATE:
		// Just stay in this state => retry
		break;
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::LOCKED;
		return;
	default:
		flowState = GSMFlowState::READY;
		break;
	}
	StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
}

void GSMSerialHandler::LaunchFlowRequest()
{
	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		// Just continue
		break;

	case GSMFlowState::TIME_REQUEST:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_TIME_CMD, strlen(GSM_TIME_CMD));
		serial->write(GSM_CMD_ASK_SYMBOL);
		break;

	case GSMFlowState::FIND_PRIMARY_PHONE:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_FIND_USER_CMD, strlen(GSM_FIND_USER_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write(QUOTATION);
		serial->write(GSM_AUX_PHONE_POSTFIX, strlen(GSM_AUX_PHONE_POSTFIX));
		serial->write(QUOTATION);
		break;

	case GSMFlowState::SIM_PIN_STATE:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_SIM_PIN_CMD, strlen(GSM_SIM_PIN_CMD));
		serial->write(GSM_CMD_ASK_SYMBOL);
		break;

	case GSMFlowState::SIM_LOGIN:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_SIM_PIN_CMD, strlen(GSM_SIM_PIN_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write(SIM_PIN_CODE, strlen(SIM_PIN_CODE));
		tryedPass = true;
		break;
	case GSMFlowState::SEND_SMS_BEGIN:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_SMS_SEND_CMD, strlen(GSM_SMS_SEND_CMD));

		serial->write(GSM_CMD_SET_SYMBOL);

		serial->write(QUOTATION);
		serial->write(primaryPhone, strlen(primaryPhone));
		serial->write(QUOTATION);
		break;
	case GSMFlowState::CALL_HANGUP:
		serial->write(GSM_CALL_HANGUP_CMD, strlen(GSM_CALL_HANGUP_CMD));
		break;
	case GSMFlowState::CALL_ANSWER:
		serial->write(GSM_CALL_ANSWER_CMD, strlen(GSM_CALL_ANSWER_CMD));
		break;
	case GSMFlowState::CALL_DIAL:
	case GSMFlowState::READY:
	default:
		return;
	}

	//FlushData();

	serial->write(CR_ASCII_SYMBOL);
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
		smsState != IncomingMessageState::NONE || 
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
	LaunchFlowRequest();

	// Wait "> " response
}

void GSMSerialHandler::CallCMD()
{
	if (IsBusy()) return;

	char *phone = callHangupStack.UnshiftFirst();

	if (phone == NULL) return;

	flowState = GSMFlowState::CALL_DIAL;
	serial->write(GSM_CALL_DIAL_CMD, strlen(GSM_CALL_DIAL_CMD));

	serial->write(phone, strlen(phone));
	free(phone);
	phone = NULL;

	serial->write(SEMICOLON_ASCII_SYMBOL);

	//FlushData();

	serial->write(CR_ASCII_SYMBOL);
	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT);
}

void GSMSerialHandler::NotifyByCallHangUp()
{
	outPrintf("NotifyByCallHangUp");
	if (smsSender[0] == 0) return;
	if (!callHangupStack.AppendCopy(smsSender)) return;

	CallCMD();
}

void GSMSerialHandler::HangupCallCMD()
{
	StopCallTimer();

	flowState = GSMFlowState::CALL_HANGUP;
	LaunchFlowRequest();
}
void GSMSerialHandler::AnswerCallCMD()
{
	StopCallTimer();

	flowState = GSMFlowState::CALL_ANSWER;
	LaunchFlowRequest();
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
	if (messageCallback(serial))
	{
		//Send message end
		serial->write(CRTLZ_ASCII_SYMBOL);
	}
	else {
		// Send message cancel callback
		serial->write(ESC_ASCII_SYMBOL);
	}
	messageCallback = NULL;

	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT);
}

bool GSMSerialHandler::LoadSymbolFromBuffer(uint8_t symbol)
{
	bool result = SerialCharResponseHandler::LoadSymbolFromBuffer(symbol);

		// Check sms text iptut extra response
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
IncomingMessageState GSMSerialHandler::SMSState()
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
