#include "GSMSerialHandler.h"

GSMSerialHandler::GSMSerialHandler(StringCallback smsCallback, Stream * serial):SerialCharResponseHandler(GSM_RESPONSE_SEPARATOR, serial)//SerialTimerResponseHandler(stream)
{
	this->smsCallback = smsCallback;
	smsSender[0] = 0;
	//primaryPhone[0] = 0;

	flowState = GSMFlowState::INITIALIZATION;
	flowTimer.Start(SERIAL_RESPONSE_TIMEOUT);


	//pinMode(6u, OUTPUT);
	//digitalWrite(6u, LOW);
}

GSMSerialHandler::~GSMSerialHandler()
{

}

void GSMSerialHandler::OnResponseReceived(bool isTimeOut, bool isOverFlow)
{
	SerialCharResponseHandler::OnResponseReceived(isTimeOut);
	size_t size = strlen(buffer);

	if (size == 0 && !isTimeOut) return;

	outPrintf("A6: [%s] [%d], type: [%d]", buffer, size, (uint8_t)flowState);

	if (isTimeOut) {
		HandleErrorResponse(buffer, size);
		return;
	}

	if (flowState == GSMFlowState::AUTH_INCOMING_MSG) {
		flowState = GSMFlowState::READY;
		HandleIncomingMessage(buffer, size);
	}
	else if (strcmp(buffer, GSM_OK_RESPONSE) == 0) {
		HandleOKResponse(buffer, size);
	}
	else if (strstr(buffer, GSM_ERROR_RESPONSE) > 0) {
		HandleErrorResponse(buffer, size);
	}
	else {
		HandleDataResponse(buffer, size);
	}

	//if (!IsBusy() && serial->available() == 0) {
	//	LaunchStateRequest();
	//}
}

void GSMSerialHandler::HandleIncomingMessage(char *response, size_t size)
{
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
	case GSMFlowState::READY:
	case GSMFlowState::WAIT_SIM_INIT:
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
		// +CMT: "+372000000",,"2019/07/25,23:25:32+03":
		// +CMT: "+37258349965","Ilja aux-1","19/07/31,13:29:43+12"

		if (strncmp(response, GSM_TS_DATA_CMD, strlen(GSM_TS_DATA_CMD)) == 0) {
			char *cmt = response + strlen(GSM_TS_DATA_CMD) + 2;
			char *cmtArgs[3];
			size_t len = SplitString(cmt, COMMA_ASCII_SYMBOL, cmtArgs, 3, false);

			// remove quotations
			ShiftQuotations(cmtArgs, len);

			char *senderName = cmtArgs[1];
			if (senderName[0] != 0) {
				if (strstr(senderName, GSM_AUX_PHONE_POSTFIX) != 0)
				{
					strcpy(smsSender, cmtArgs[0]);
					flowState = GSMFlowState::AUTH_INCOMING_MSG;
					return;
				}
			}

			flowState = GSMFlowState::NOT_AUTH_INCOMING_MSG;
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
		flowState = GSMFlowState::FIND_PRIMARY_PHONE;
		break;
	case GSMFlowState::FIND_PRIMARY_PHONE:
		outPrintf("GSM READY! Primary phone: %s", primaryPhone);
		//break;
	case GSMFlowState::SEND_SMS_FLOW:
		flowState = GSMFlowState::READY;
		break;
	}

	flowTimer.Start(SERIAL_RESPONSE_TIMEOUT);
}

void GSMSerialHandler::HandleErrorResponse(char *response, size_t size)
{
	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
		break;
	case GSMFlowState::SHORT_RESPONSE:
	case GSMFlowState::MSG_TEXT_MODE:
	case GSMFlowState::SIM_PIN_STATE:
	case GSMFlowState::SIM_LOGIN:
		flowState = GSMFlowState::LOCKED;
		return;
	case GSMFlowState::SEND_SMS_BEGIN:
	case GSMFlowState::SEND_SMS_FLOW:
	case GSMFlowState::FIND_PRIMARY_PHONE:
		flowState = GSMFlowState::READY;
		break;
	default:
		break;
	}
	flowTimer.Start(SERIAL_RESPONSE_TIMEOUT);
}

void GSMSerialHandler::Loop()
{
	SerialCharResponseHandler::Loop();
	
	bool wasLaunchedFT = flowTimer.IsRunning();
	flowTimer.Loop();

	if (wasLaunchedFT && !flowTimer.IsRunning()) {
		LaunchStateRequest();
	}
}

void GSMSerialHandler::LaunchStateRequest()
{
	//char resultRequest[32];
	//resultRequest[0] = 0;
	//strcat(resultRequest, GSM_INIT_CMD);

	switch (flowState)
	{
	case GSMFlowState::INITIALIZATION:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		// Just continue
		break;
	case GSMFlowState::SHORT_RESPONSE:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_SHORT_RESPONSE, strlen(GSM_SHORT_RESPONSE));
		break;
	case GSMFlowState::REG_GSM_SERVICE:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_REG_CMD, strlen(GSM_REG_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write('1');
		break;
	case GSMFlowState::MSG_TEXT_MODE:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_MSG_TEXT_CMD, strlen(GSM_MSG_TEXT_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write('1');
		break;
	case GSMFlowState::MSG_INCOMING_FORMAT:
		serial->write(GSM_INIT_CMD, strlen(GSM_INIT_CMD));
		serial->write(GSM_MSG_ARRIVE_CMD, strlen(GSM_MSG_ARRIVE_CMD));
		serial->write(GSM_CMD_SET_SYMBOL);
		serial->write("1,2,0,0,0", 9);
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
	case GSMFlowState::READY:
		//outPrintf("GSM Ready!");
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

void GSMSerialHandler::FinalizeSendMessage()
{
	if (messageCallback(serial))
	{
		//Send message end
		serial->write(LF_ASCII_SYMBOL);
	}
	else {
		// Send message cancel callback
		serial->write(ESC_ASCII_SYMBOL);
	}
	StartTimeoutTimer(SERIAL_RESPONSE_TIMEOUT);
}

bool GSMSerialHandler::IsNetworkConnected()
{
	return cRegState == 1u || cRegState == 5u;
}
bool GSMSerialHandler::IsRoaming()
{
	return cRegState == 5u;
}
