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
    char *primaryPhone = phoneBookReader.GetPrimaryPhone();
    if (primaryPhone[0] == 0) {
        return;
    }
    GSMSerialHandler::SendSMSMessage(messageCallback, primaryPhone);
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
		flowState = UbloxFlowState::ENABLE_DTMF_EVENT;
		break;
	case UbloxFlowState::ENABLE_DTMF_EVENT:
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
		flowState = UbloxFlowState::READ_PHONE_BOOK;
		break;
	case UbloxFlowState::READ_PHONE_BOOK:
		flowState = UbloxFlowState::READY;
		break;
    case UbloxFlowState::READY:
        if (this->response != NULL) {
            GSMSerialHandler::HandleDataResponse(reqCmd, this->response, size);
            FlushResponse();
        }
        return;
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
    if (strncmp(response, GSM_CALL_STATE_CMD, strlen(GSM_CALL_STATE_CMD)) == 0) {
        SaveResponse(response);
        return;
    }
    if (strncmp(response, UBLOX_DTMF_EVENT, strlen(UBLOX_DTMF_EVENT)) == 0) {
		// +UDTMFD: 1
		char *pCode = response + strlen(UBLOX_DTMF_EVENT) + 2;
		HandleDtfm(pCode[0]);
        return;
	} 
    if (strncmp(response, GSM_READ_PHONE_BOOK_CMD, strlen(GSM_READ_PHONE_BOOK_CMD)) == 0) {
	    // +CPBR: 1,\"+372111111\",145,\"1 aux-1"
		char *cpbf = response + strlen(GSM_FIND_USER_CMD) + 1;
		char *cpbfArgs[4];
		SplitString(cpbf, ',', cpbfArgs, 4, false);
		// remove quotations
		ShiftQuotations(cpbfArgs, 4);

		char *userName = cpbfArgs[3];
        char *phone = cpbfArgs[1];

        phoneBookReader.HandleEntriy(phone, userName);

        return;
	}
    if (strncmp(response, GSM_UCALLSTAT, strlen(GSM_UCALLSTAT)) == 0) {

		char *clccContent = response + strlen(GSM_UCALLSTAT) + 2;
		char *clccArgs[2];
		SplitString(clccContent, ',', clccArgs, 8, false);

		uint8_t callStateIndex = atoi(clccArgs[1]);
        bool isIncoming = false;
        bool needFetch = false;

        switch (callStateIndex)
        {
            case GSMCallState::ACTIVE:
                needFetch = true;
                break;
            case GSMCallState::HOLD:
                needFetch = true;
                break;

            // Outgoing
            case GSMCallState::DIALING:
                break;
            case GSMCallState::ALERTING:
                break;

            // Incoming
            case GSMCallState::INCOMING:
                isIncoming = true;
                needFetch = true;
                break;
            case GSMCallState::WAITING:
                isIncoming = true;
                break;

            case GSMCallState::DISCONNECT:
                break;
        
        }

        // callType
        // 0 - Incoming
        // 1 - outgoing

        // Outgoing: 2->3->0->6
        // 0 = accepted ongoing call
        // Incoming:
        // 4->0->6

        if (needFetch) {
            WriteGsmSerial(GSM_CLCC, false, false);
        } else {
            UpdateCallState(isIncoming, callStateIndex, NULL, NULL);
        }
        return;
    }
    GSMSerialHandler::HandleDataResponse(reqCmd, response, size);
}

bool UbloxGSMHandler::IsAuthorized(char *number, char *entryName)
{
    return phoneBookReader.HasNumber(number);
}

void UbloxGSMHandler::OnFlowTimer()
{
    if (debugPrint != NULL) {
        debugPrint->print("UbloxGSMHandler::OnFlowTimer: ");
        debugPrint->println((uint8_t)flowState, 10);
    }

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
	case UbloxFlowState::ENABLE_DTMF_EVENT:
        WriteGsmSerial(UBLOX_DTMF_CMD, false, true, (char *)"1,1");
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
        phoneBookReader.Clear();
        WriteGsmSerial(GSM_READ_PHONE_BOOK_CMD, false, true, (char *)"1,254");
		break;

    default:
        break;
    }
}


void UbloxGSMHandler::FlushResponse()
{
    if (response == NULL) return;
    free(response);
    response = NULL;
}

void UbloxGSMHandler::SaveResponse(char *response)
{
    FlushResponse();
    this->response = (char *)malloc(strlen(response)+1);
    strcpy(this->response, response);
}