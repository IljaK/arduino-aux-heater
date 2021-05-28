#include "SimcomGSMHandler.h"


SimcomGSMHandler::SimcomGSMHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial):GSMSerialHandler(smsCallback, dtmfCallback, serial)
{
    primaryPhone[0] = 0;
}

SimcomGSMHandler::~SimcomGSMHandler()
{

}
void SimcomGSMHandler::Start()
{

}

void SimcomGSMHandler::OnNetworkStateUpdated()
{
    if (IsNetworkConnected() && flowState == SimcomFlowState::REG_NETWORK) {
        HandleOKResponse(NULL, NULL, 0);
    } 
}

void SimcomGSMHandler::SendSMSMessage(StreamCallback messageCallback)
{
    if (primaryPhone[0] == 0) {
        return;
    }
    GSMSerialHandler::SendSMSMessage(messageCallback, primaryPhone);
    // Send sms to primary phone
}

void SimcomGSMHandler::HandleOKResponse(char * reqCmd, char *response, size_t size)
{
	switch (flowState)
	{
	case SimcomFlowState::INITIALIZATION:
		flowState = SimcomFlowState::SIM_PIN_STATE;
		break;
	case SimcomFlowState::SIM_PIN_STATE:
	{
		switch (SimPinState())
		{
		case GSMSimPinState::SIM_PIN_STATE_PIN:
			flowState = SimcomFlowState::SIM_LOGIN;
			break;
		case GSMSimPinState::SIM_PIN_STATE_READY:
			flowState = SimcomFlowState::REG_NETWORK;
			break;
		default:
			flowState = SimcomFlowState::LOCKED;
			return;
		}
		break;
	}
	case SimcomFlowState::SIM_LOGIN:
		flowState = SimcomFlowState::REG_NETWORK;
		break;
	case SimcomFlowState::REG_NETWORK:
		flowState = SimcomFlowState::FIND_PRIMARY_PHONE;
		break;

	case SimcomFlowState::FIND_PRIMARY_PHONE:
        if (debugPrint != NULL) {
            debugPrint->print(F("Primary phone: "));
            debugPrint->println(primaryPhone);
        }
		flowState = SimcomFlowState::TIME_REQUEST;
		break;
	case SimcomFlowState::TIME_REQUEST:
		flowState = SimcomFlowState::READY;
		break;
	default:
		return;
	}
	StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
}

void SimcomGSMHandler::HandleErrorResponse(char * reqCmd, char *response, size_t size)
{
	switch (flowState)
	{
	case SimcomFlowState::INITIALIZATION:
		// Just stay in this state => retry
		break;
	case SimcomFlowState::SIM_PIN_STATE:
	case SimcomFlowState::SIM_LOGIN:
		flowState = SimcomFlowState::LOCKED;
		return;
	default:
		// TODO: Make few attemts then go to error or ready
		flowState = SimcomFlowState::READY;
		break;
	}
	StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
}

/*
void SimcomGSMHandler::SendFlowCMD(char *data)
{
   switch (flowState)
	{
	case SimcomFlowState::INITIALIZATION:
        WriteGsmSerial(NULL);
		break;

	case SimcomFlowState::TIME_REQUEST:
        WriteGsmSerial((char *)GSM_TIME_CMD, true);
		break;

	case SimcomFlowState::REG_NETWORK:
        WriteGsmSerial((char*)GSM_FIND_USER_CMD, false, true, data, true);
		break;

	case SimcomFlowState::FIND_PRIMARY_PHONE:
        WriteGsmSerial((char*)GSM_REG_CMD, false, true, (char*)GSM_AUX_PHONE_POSTFIX, true);
		break;

	case SimcomFlowState::SIM_PIN_STATE:
        WriteGsmSerial((char*)GSM_SIM_PIN_CMD, true);
		break;

	case SimcomFlowState::SIM_LOGIN:
        WriteGsmSerial((char*)GSM_SIM_PIN_CMD, false, true, (char*)SIM_PIN_CODE, true);
		break;
	case SimcomFlowState::SEND_SMS_BEGIN:
        WriteGsmSerial((char*)GSM_SMS_SEND_CMD, false, true, primaryPhone, true);
		break;
	case SimcomFlowState::CALL_DIAL:
        WriteGsmSerial((char*)GSM_CALL_DIAL_CMD, false, true, data, false, true);
        break;
	case SimcomFlowState::CALL_HANGUP:
        WriteGsmSerial((char*)GSM_CALL_HANGUP_CMD);
		break;
	case SimcomFlowState::CALL_ANSWER:
        WriteGsmSerial((char*)GSM_CALL_ANSWER_CMD);
		break;
	default:
		break;
	}
}
*/

void SimcomGSMHandler::HandleDataResponse(char * reqCmd, char *response, size_t size)
{
    if (strncmp(response, GSM_FIND_USER_CMD, strlen(GSM_FIND_USER_CMD)) == 0) {
	    // +CPBF: 1,\"+372111111\",145,\"1 aux-1"
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
		if (lowestIndex >= index) {
			lowestIndex = index;
			strcpy(primaryPhone, cpbfArgs[1]);
		}
	} else if (strncmp(response, SIMCOM_DTMF_CMD, strlen(SIMCOM_DTMF_CMD)) == 0) {
		// +DTMF: 1
		char *pCode = response + strlen(SIMCOM_DTMF_CMD) + 2;
		HandleDtfm(pCode[0]);
	} else {
        GSMSerialHandler::HandleDataResponse(reqCmd, response, size);
    }
}

bool SimcomGSMHandler::IsAuthorized(char *number, char *entryName)
{
	return entryName != NULL && entryName[0] != 0 && strstr(entryName, GSM_AUX_PHONE_POSTFIX) != NULL;
}


void SimcomGSMHandler::OnFlowTimer()
{
    
}