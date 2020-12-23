#pragma once
#include "GSMSerialHandler.h"
#include "PhonebookReader.h"

enum class UbloxFlowState : uint8_t
{
	INITIALIZATION,

    ENABLE_CREG_EVENT,
    ENABLE_UCALLSTAT_EVENT,

	SIM_PIN_STATE,
	SIM_LOGIN,

    CHECK_REG_NETWORK,
    WAIT_REG_NETWORK,

    SYNC_TIME, // For sync time after reg state change

    TIME_REQUEST,
    GET_PHONE_BOOK_ENTRIES,
    READ_PHONE_BOOK,

	READY,
	LOCKED
};


constexpr char GSM_UCALLSTAT[] = "+UCALLSTAT"; // Enable call status events
constexpr char GSM_CLCC[] = "+CLCC"; // Call state info

class UbloxGSMHandler : public GSMSerialHandler
{
public:
    UbloxGSMHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial);
    virtual ~UbloxGSMHandler();
	void SendSMSMessage(StreamCallback messageCallback);
    void Start() override;
private:
    UbloxFlowState flowState = UbloxFlowState::INITIALIZATION;
    PhonebookReader phoneBookReader;

protected:
    void HandleErrorResponse(char * reqCmd, char *response, size_t size) override;
    void HandleDataResponse(char * reqCmd, char *response, size_t size) override;
    void HandleOKResponse(char * reqCmd, char *response, size_t size) override;
    bool IsAuthorized(char *number, char *entryName) override;
    void OnFlowTimer() override;
    void OnNetworkStateUpdated() override;
};