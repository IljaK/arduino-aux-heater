#include "gsm/SimcomGSMHandler.h"
#include "SerialStream.h"

class SimcomGSMTestMock: public SimcomGSMHandler
{
public: 
	SerialStream * serialStream;

	SimcomGSMTestMock(SerialStream * serialStream, SMSCallback smsCallback = NULL, DTMFCallback dtmfCallback = NULL);
	~SimcomGSMTestMock();

	void BeginInitialization();
	void FinalizeInitialization();

	void ReadResponse(char * response);

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;

    SimcomFlowState FlowState() { return flowState; }
};