#include "UbloxGSMHandler.h"


UbloxGSMHandler::UbloxGSMHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial):GSMSerialHandler(smsCallback, dtmfCallback, serial)
{
    
}

UbloxGSMHandler::~UbloxGSMHandler()
{

}

void UbloxGSMHandler::Start()
{
    StartFlowTimer(SERIAL_RESPONSE_TIMEOUT);
}

void UbloxGSMHandler::SendSMSMessage(StreamCallback messageCallback)
{
    // TODO:
    //if (primaryPhone[0] == 0) {
    //    return;
    //}
    //GSMSerialHandler::SendSMSMessage(messageCallback, primaryPhone);
    // Send sms to primary phone
}

void UbloxGSMHandler::OnNetworkStateUpdated() 
{
    switch (flowState)
	{
        case UbloxFlowState::READY:
            if (IsNetworkConnected()) {
                flowState = UbloxFlowState::SYNC_TIME;
            }
            StartFlowTimer(GSM_CMD_DELAY);
            break;
        case UbloxFlowState::WAIT_REG_NETWORK:
            if (IsNetworkConnected()) {
                flowState = UbloxFlowState::TIME_REQUEST;
            }
            StartFlowTimer(GSM_CMD_DELAY);
            break;
        default:
            break;
    }
}

void UbloxGSMHandler::HandleOKResponse(char * reqCmd, char *response, size_t size)
{
	switch (flowState)
	{
	case UbloxFlowState::INITIALIZATION:
		flowState = UbloxFlowState::ENABLE_CREG_EVENT;
		break;

	case UbloxFlowState::ENABLE_CREG_EVENT:
		flowState = UbloxFlowState::ENABLE_UCALLSTAT_EVENT;
		break;
	case UbloxFlowState::ENABLE_UCALLSTAT_EVENT:
		flowState = UbloxFlowState::SIM_PIN_STATE;
		break;

	case UbloxFlowState::SIM_PIN_STATE:
	{
		switch (SimPinState())
		{
		case GSMSimPinState::SIM_PIN_STATE_PIN:
			flowState = UbloxFlowState::SIM_LOGIN;
			break;
		case GSMSimPinState::SIM_PIN_STATE_READY:
			flowState = UbloxFlowState::CHECK_REG_NETWORK;
			break;
		default:
			flowState = UbloxFlowState::LOCKED;
			return;
		}
		break;
	}
	case UbloxFlowState::SIM_LOGIN:
		flowState = UbloxFlowState::WAIT_REG_NETWORK;
		break;
	case UbloxFlowState::CHECK_REG_NETWORK:
		if (IsNetworkConnected()) {
            flowState = UbloxFlowState::TIME_REQUEST;
        }
		break;
	case UbloxFlowState::SYNC_TIME:
		flowState = UbloxFlowState::READY;
		return;
	case UbloxFlowState::TIME_REQUEST:
		flowState = UbloxFlowState::GET_PHONE_BOOK_ENTRIES;
		break;
	case UbloxFlowState::GET_PHONE_BOOK_ENTRIES:
		flowState = UbloxFlowState::READ_PHONE_BOOK;
		break;
    default:
        break;
	}
	StartFlowTimer(GSM_CMD_DELAY);
}

void UbloxGSMHandler::HandleErrorResponse(char * reqCmd, char *response, size_t size)
{
	switch (flowState)
	{
	case UbloxFlowState::INITIALIZATION:
		// Just stay in this state => retry
		break;
	case UbloxFlowState::SIM_PIN_STATE:
	case UbloxFlowState::SIM_LOGIN:
		flowState = UbloxFlowState::LOCKED;
		return;
	default:
		// TODO: Make few attemts then go to error or ready
		flowState = UbloxFlowState::READY;
		break;
	}
	StartFlowTimer(GSM_CMD_DELAY);
}

void UbloxGSMHandler::HandleDataResponse(char * reqCmd, char *response, size_t size)
{
    if (strncmp(response, GSM_FIND_USER_CMD, strlen(GSM_FIND_USER_CMD)) == 0) {
	    // +CPBF: 1,\"+372111111\",145,\"1 aux-1"
		char *cpbf = response + strlen(GSM_FIND_USER_CMD) + 1;
		char *cpbfArgs[4];
		SplitString(cpbf, ',', cpbfArgs, 4, false);
		// remove quotations
		ShiftQuotations(cpbfArgs, 4);

		char *userName = cpbfArgs[3];
        char *phone = cpbfArgs[1];

        phoneBookReader.HandleEntriy(phone, userName);

        return;
	} else if (strncmp(response, GSM_UCALLSTAT, strlen(GSM_UCALLSTAT)) == 0) {
        //if (flowState == UbloxFlowState::READY) {
            WriteGsmSerial(GSM_CLCC, false, false);
        //}
        return;
    }
    GSMSerialHandler::HandleDataResponse(reqCmd, response, size);
}

bool UbloxGSMHandler::IsAuthorized(char *number, char *entryName)
{
	return entryName != NULL && entryName[0] != 0 && strstr(entryName, GSM_AUX_PHONE_POSTFIX) != NULL;
}


void UbloxGSMHandler::OnFlowTimer()
{
    //DebugHandler::outWrite("UbloxGSMHandler::OnFlowTimer: ", true);
    //DebugHandler::outWriteASCII((uint8_t)flowState, 10, true);
    //DebugHandler::outWrite("\r\n", true);

    switch (flowState)
	{
	case UbloxFlowState::INITIALIZATION:
        WriteGsmSerial(NULL);
		break;
	case UbloxFlowState::ENABLE_CREG_EVENT:
        WriteGsmSerial(GSM_REG_CMD, false, true, (char *)"1");
		break;
	case UbloxFlowState::ENABLE_UCALLSTAT_EVENT:
        WriteGsmSerial(GSM_UCALLSTAT, false, true, (char *)"1");
		break;
	case UbloxFlowState::SIM_PIN_STATE:
        WriteGsmSerial(GSM_SIM_PIN_CMD, true);
		break;

	case UbloxFlowState::SIM_LOGIN:
        WriteGsmSerial(GSM_SIM_PIN_CMD, false, true, (char*)SIM_PIN_CODE, true);
		break;
	case UbloxFlowState::CHECK_REG_NETWORK:
        WriteGsmSerial(GSM_REG_CMD, true);
		break;
    case UbloxFlowState::SYNC_TIME:
	case UbloxFlowState::TIME_REQUEST:
        WriteGsmSerial(GSM_TIME_CMD, true);
		break;
	case UbloxFlowState::READ_PHONE_BOOK:
        // TODO:
		break;

    default:
        break;
    }
}