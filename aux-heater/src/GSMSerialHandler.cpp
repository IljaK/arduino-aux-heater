#include "GSMSerialHandler.h"

GSMSerialHandler::GSMSerialHandler(StringCallback smsCallback, Stream * serial):SerialCharResponseHandler(GSM_RESPONSE_SEPARATOR, serial)//SerialTimerResponseHandler(stream)
{
	this->smsCallback = smsCallback;
	smsSender[0] = 0;
	//primaryPhone[0] = 0;

	flowState = GSMFlowState::INITIALIZATION;
	flowTimer = Timer::Start(this, SERIAL_RESPONSE_TIMEOUT);
}

GSMSerialHandler::~GSMSerialHandler()
{
	if (flowTimer != 0) Timer::Stop(flowTimer);
	flowTimer = 0;
}
void GSMSerialHandler::OnTimerComplete(TimerID timerId)
{
	if (hangUpTimer == timerId) {
		hangUpTimer = 0;
		HangupCallCMD();
	} else if (timerId == flowTimer) {
		flowTimer = 0;
		LaunchStateRequest();
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

	if (request) {
		outPrintf("Received: [request: %s] [%s] [%d], type: [%d]", request, buffer, size, (uint8_t)flowState);
	} else {
		outPrintf("Received: [%s] [%d], type: [%d]", buffer, size, (uint8_t)flowState);
	}

	if (isTimeOut) {
		HandleErrorResponse(buffer, size);
		request = NULL;
		return;
	}

	switch (messageState)
	{
	case IncomingMessageState::NONE:
		// Nothing to do here
		break;
	case IncomingMessageState::AUTH_INCOMING_MSG:
		messageState = IncomingMessageState::NONE;
		HandleIncomingMessage(buffer, size);
		return;
	case IncomingMessageState::NOT_AUTH_INCOMING_MSG:
		messageState = IncomingMessageState::NONE;
		// Skip command
		return;
	}	

	if (strcmp(buffer, GSM_OK_RESPONSE) == 0) {
		request = NULL;
		HandleOKResponse(buffer, size);
	}
	else if (strstr(buffer, GSM_ERROR_RESPONSE) > 0) {
		request = NULL;
		HandleErrorResponse(buffer, size);
	}
	else {
		//request = NULL;
		HandleDataResponse(buffer, size);
	}
}

void GSMSerialHandler::HandleIncomingMessage(char *response, size_t size)
{
	outPrintf("Handle SMS: [%s] sender: [%s]", response, smsSender);
	if (smsCallback) smsCallback(response, size);
}

void GSMSerialHandler::HandleDataResponse(char *response, size_t size)
{
	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
	{
		break;
	}
	case GSMFlowState::FIND_PRIMARY_PHONE:
	{
		if (strncmp(response, GSM_FIND_USER_CMD, strlen(GSM_FIND_USER_CMD)) == 0) 
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
		break;
	}
	case GSMFlowState::SEND_SMS_BEGIN:
	{
		flowState = GSMFlowState::SEND_SMS_FLOW;
		FinalizeSendMessage();
		break;
	}
	case GSMFlowState::SIM_PIN_STATE:
	case GSMFlowState::SIM_PIN_STATE_READY:
	case GSMFlowState::SIM_PIN_STATE_PIN:
	case GSMFlowState::SIM_PIN_STATE_UNKNOWN:
	{
		size_t cmdLen = strlen(GSM_SIM_PIN_CMD);
		if (strncmp(response, GSM_SIM_PIN_CMD, cmdLen) != 0) {
			break;
		}

		char *simState = response + cmdLen + 2;

		outPrintf(simState);

		if (strncmp(GSM_SIM_STATE_READY, simState, strlen(GSM_SIM_STATE_READY)) == 0) {
			flowState = GSMFlowState::SIM_PIN_STATE_READY;
		}
		else if (strncmp(GSM_SIM_STATE_SIM_PIN, simState, strlen(GSM_SIM_STATE_SIM_PIN)) == 0 && !tryedPass) {
			flowState = GSMFlowState::SIM_PIN_STATE_PIN;
		}
		else {
			flowState = GSMFlowState::SIM_PIN_STATE_UNKNOWN;
		}
		break;
	}
	case GSMFlowState::WAIT_SIM_INIT:
	case GSMFlowState::READY:
	{
		// +CIEV
		/*
		if (strncmp(response, GSM_EVENT_DATA_CMD, strlen(GSM_EVENT_DATA_CMD)) == 0) {

			// [+CTZV:19/07/28, 10:45:57, +03] [27]

			char *args[2];
			uint8_t argsLength = SplitString(response, COMMA, args, 2, false);

			if (args[0] == 0) return;

			// Shift space + : signs = 2
			char *eventArg = args[0] + strlen(GSM_EVENT_DATA_CMD) + 2;

			// [+CIEV: "MESSAGE", 1][18]
			if (strncmp(eventArg + 1, GSM_EVENT_DATA_MESSAGE, strlen(GSM_EVENT_DATA_MESSAGE)) == 0) {
				
			}
			// [+CIEV: service, 1] [18]
			if (strncmp(eventArg, GSM_EVENT_DATA_SERVICE, strlen(GSM_EVENT_DATA_SERVICE)) == 0) {
				isService = atoi(args[1]) == 1;
			}
			// [+CIEV: roam, 0]
			else if (strncmp(eventArg, GSM_EVENT_DATA_ROAMING, strlen(GSM_EVENT_DATA_ROAMING)) == 0) {
				isRoaming = atoi(args[1]) == 1;
				if (flowState == GSMFlowState::WAIT_SIM_INIT) {
					flowState = GSMFlowState::MSG_TEXT_MODE;
				}
			}
			
		}*/
		// +CMT: "+372000000",,"2019/07/25,23:25:32+03"
		// +CMT: "+32353","2356 aux-1","19/07/31,13:29:43+12"

		if (strncmp(response, GSM_TS_DATA_CMD, strlen(GSM_TS_DATA_CMD)) == 0) {
			char *cmt = response + strlen(GSM_TS_DATA_CMD) + 2;
			char *cmtArgs[3];
			size_t len = SplitString(cmt, COMMA_ASCII_SYMBOL, cmtArgs, 3, false);

			// remove quotations
			ShiftQuotations(cmtArgs, len);

			char *senderName = cmtArgs[1];
			if (senderName[0] != 0 && strstr(senderName, GSM_AUX_PHONE_POSTFIX) != 0) {
				strcpy(smsSender, cmtArgs[0]);
				messageState = IncomingMessageState::AUTH_INCOMING_MSG;
			} else {
				messageState = IncomingMessageState::NOT_AUTH_INCOMING_MSG;
			}
		}
		else if (strncmp(response, GSM_REG_CMD, strlen(GSM_REG_CMD)) == 0) {
			// +CREG: 1,2
			char *creg = response + strlen(GSM_REG_CMD) + 2;
			char *cregArgs[2];

			SplitString(creg, COMMA_ASCII_SYMBOL, cregArgs, 2, false);
			cRegState = atoi(cregArgs[1]);
		}
		else if (strcmp(response, GSM_SIM_AUTH_READY) == 0) {
			if (flowState == GSMFlowState::WAIT_SIM_INIT) {
				flowState = GSMFlowState::MSG_TEXT_MODE;
				LaunchStateRequest();
			}
		}
		else if (strncmp(response, GSM_SMS_SEND_CMD, strlen(GSM_SMS_SEND_CMD)) == 0) {

			// +CMGS: 12
			//char *cLength = response + strlen(GSM_SMS_SEND_CMD) + 2;
			//uint8_t messageLength = atoi(cLength);
			//outPrintf("Message length: %s", cLength);
		}
		break;
	}
	case GSMFlowState::CALL_PROGRESS:
		if (strncmp(response, GSM_CALL_STATE_CMD, strlen(GSM_CALL_STATE_CMD)) == 0) {
			// +CLCC: 1,0,3,0,0,"+37211111",145,"3123 aux-1"
			char *clccContent = response + strlen(GSM_CALL_STATE_CMD) + 2;
			char *clccArgs[8];
			size_t len = SplitString(clccContent, COMMA_ASCII_SYMBOL, clccArgs, 8, false);

			if (atoi(clccArgs[2]) == 3 ) {
				// Send cancel call CMD
				//HangupCallCMD();
				hangUpTimer = Timer::Start(this, CALL_WAIT_DURATION);
			} else if (atoi(clccArgs[2]) == 6 ) {
				if (hangUpTimer != 0) {
					HangupCallCMD();
				}
			}
		}
		break;
	default:
		break;
	}
}

void GSMSerialHandler::HandleOKResponse(char *response, size_t size)
{
	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
		flowState = GSMFlowState::SHORT_RESPONSE;
		//flowState = GSMFlowState::REG_GSM_SERVICE;
		break;
	case GSMFlowState::SHORT_RESPONSE:
		flowState = GSMFlowState::REG_GSM_SERVICE;
		break;

	case GSMFlowState::REG_GSM_SERVICE:
		flowState = GSMFlowState::SIM_PIN_STATE;
		break;
	case GSMFlowState::SIM_PIN_STATE:
		break;

	case GSMFlowState::SIM_PIN_STATE_READY:
		flowState = GSMFlowState::MSG_TEXT_MODE;
		break;

	case GSMFlowState::SIM_PIN_STATE_PIN:
		flowState = GSMFlowState::SIM_LOGIN;
		break;
	case GSMFlowState::SIM_PIN_STATE_UNKNOWN:
		flowState = GSMFlowState::LOCKED;
		break;
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::WAIT_SIM_INIT;
		return;
		break;
	case GSMFlowState::MSG_TEXT_MODE:
		flowState = GSMFlowState::MSG_INCOMING_FORMAT;
		break;
	case GSMFlowState::MSG_INCOMING_FORMAT:
		flowState = GSMFlowState::CALL_STATE_INFO;
		break;
	case GSMFlowState::CALL_STATE_INFO:
		flowState = GSMFlowState::FIND_PRIMARY_PHONE;
		break;
	case GSMFlowState::FIND_PRIMARY_PHONE:
		outPrintf("GSM READY! Primary phone: %s", primaryPhone);
		//break;
	case GSMFlowState::SEND_SMS_FLOW:
	case GSMFlowState::CALL_HANGUP:
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
	case GSMFlowState::SHORT_RESPONSE:
	case GSMFlowState::MSG_TEXT_MODE:
		// Just stay in this state => retry
		break;
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::LOCKED;
		return;
	case GSMFlowState::SEND_SMS_BEGIN:
	case GSMFlowState::SEND_SMS_FLOW:
	case GSMFlowState::FIND_PRIMARY_PHONE:
	case GSMFlowState::CALL_PROGRESS:
	case GSMFlowState::CALL_HANGUP:
		flowState = GSMFlowState::READY;
		break;
	default:
		break;
	}
	StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
}

void GSMSerialHandler::LaunchStateRequest()
{
	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
		request = GSM_INIT_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		// Just continue
		break;
	case GSMFlowState::SHORT_RESPONSE:
		request = GSM_SHORT_RESPONSE;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_SHORT_RESPONSE, strlen(GSM_SHORT_RESPONSE));
		break;

	case GSMFlowState::REG_GSM_SERVICE:
		request = GSM_REG_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_REG_CMD, strlen(GSM_REG_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write('1');
		break;

	case GSMFlowState::MSG_TEXT_MODE:
		request = GSM_MSG_TEXT_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_MSG_TEXT_CMD, strlen(GSM_MSG_TEXT_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write('1');
		break;

	case GSMFlowState::MSG_INCOMING_FORMAT:
		request = GSM_MSG_ARRIVE_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_MSG_ARRIVE_CMD, strlen(GSM_MSG_ARRIVE_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write("2,2,0,0,0", 9);
		break;

	case GSMFlowState::CALL_STATE_INFO:
		request = GSM_CALL_STATE_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_CALL_STATE_CMD, strlen(GSM_CALL_STATE_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write('1');
		break;

	case GSMFlowState::FIND_PRIMARY_PHONE:
		request = GSM_FIND_USER_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_FIND_USER_CMD, strlen(GSM_FIND_USER_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write(QUOTATION);
		serial->write(GSM_AUX_PHONE_POSTFIX, strlen(GSM_AUX_PHONE_POSTFIX));
		serial->write(QUOTATION);
		break;

	case GSMFlowState::SIM_PIN_STATE:
		request = GSM_SIM_PIN_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_SIM_PIN_CMD, strlen(GSM_SIM_PIN_CMD));
		serial->write(GSM_CMD_ASK_SYMBOL);
		break;

	case GSMFlowState::SIM_LOGIN:
		request = GSM_SIM_PIN_CMD;
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_SIM_PIN_CMD, strlen(GSM_SIM_PIN_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write(SIM_PIN_CODE, strlen(SIM_PIN_CODE));
		tryedPass = true;
		break;

	case GSMFlowState::READY:
		return;
	default:
		return;
	}

	FlushData();

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
	return SerialCharResponseHandler::IsBusy() || flowState != GSMFlowState::READY;
}

void GSMSerialHandler::SendSMSMessage(StreamCallback messageCallback)
{
	// TODO: If busy, add to stack and send on ready

	if (messageCallback == NULL) return;
	if (IsBusy() || primaryPhone[0] == 0) return;

	this->messageCallback = messageCallback;
	flowState = GSMFlowState::SEND_SMS_BEGIN;
	// AT+CMGS=\"+ZZxxxxxxxxxx\""
	serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
	serial->write(GSM_SMS_SEND_CMD, strlen(GSM_SMS_SEND_CMD));

	serial->write(GSM_CMD_SET_SYMBOL);

	serial->write(QUOTATION);
	serial->write(primaryPhone, strlen(primaryPhone));
	serial->write(QUOTATION);

	serial->write(CR_ASCII_SYMBOL);
	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT);

	// Wait "> " response
}

void GSMSerialHandler::NotifyByCallHangUp()
{
	// TODO: If busy, add to stack and call on ready
	if (IsBusy()) return;
	if (smsSender[0] == 0) return;

	flowState = GSMFlowState::CALL_PROGRESS;
	serial->write(GSM_CALL_CMD, strlen(GSM_CALL_CMD));
	serial->write(smsSender, strlen(smsSender));
	serial->write(SEMICOLON_ASCII_SYMBOL);
	serial->write(CR_ASCII_SYMBOL);
	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT * 10u);
}

void GSMSerialHandler::HangupCallCMD()
{
	if (hangUpTimer != 0) {
		Timer::Stop(hangUpTimer);
		hangUpTimer = 0;
	}
	flowState = GSMFlowState::CALL_HANGUP;
	serial->write(GSM_CALL_BREAK_CMD, strlen(GSM_CALL_BREAK_CMD));
	serial->write(CR_ASCII_SYMBOL);
	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT);
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
