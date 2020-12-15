#include "GSMSerialHandler.h"

enum class SimcomFlowState : uint8_t
{
	INITIALIZATION,

	SIM_PIN_STATE,
	SIM_LOGIN,

    REG_NETWORK,
	FIND_PRIMARY_PHONE,

	READY,
	LOCKED,

	TIME_REQUEST,
};

class SimcomGSMHandler : public GSMSerialHandler
{
public:
    SimcomGSMHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial);
    virtual ~SimcomGSMHandler();
	void SendSMSMessage(StreamCallback messageCallback);
    void Start() override;
private:
	char primaryPhone[18] = "";
	uint8_t lowestIndex = 255;
    SimcomFlowState flowState = SimcomFlowState::INITIALIZATION;

protected:
    void HandleErrorResponse(char * reqCmd, char *response, size_t size) override;
    void HandleDataResponse(char * reqCmd, char *response, size_t size) override;
    void HandleOKResponse(char * reqCmd, char *response, size_t size) override;
    bool IsAuthorized(char *number, char *entryName) override;
    void OnFlowTimer() override;
    void OnNetworkStateUpdated() override;
};