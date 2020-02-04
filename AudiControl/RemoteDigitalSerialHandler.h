#include "Util.h"
#include "SoftwareSerialHandler.h"
#include "SerialTimerResponseHandler.h"

enum class RemoteFlowState: uint8_t
{
	NONE,
	SET_WAIT_LOW,
	WAIT_RESPONSE,
	SET_WAIT_COMPLETE
};

class RemoteDigitalSerialHandler: public SerialTimerResponseHandler
{
public:
	RemoteDigitalSerialHandler(SoftwareSerialHandler *serial);
	~RemoteDigitalSerialHandler();
	
	void SwitchBaudRate(char *serialBaudRate);
	void Loop() override;
	bool IsBusy() override;
	bool isSetSuccess = false;
	void SendATCommand(char * command, uint8_t size);

	void OnResponseReceived(bool IsTimeOut) override;

private:

	Timer flowTimer;
	RemoteFlowState flowState = RemoteFlowState::NONE;
	char * setCommand = NULL;
	uint8_t baudRateIndex = 0;

	void HandleFlowState();
	bool IsSetSucceeded();


	void OnATResponse();

	// 1 - set pin to HIHG + wait 200 msec
	// 2 - send command
	// 3 - set pin to low + wait 200 msec
protected:

};

