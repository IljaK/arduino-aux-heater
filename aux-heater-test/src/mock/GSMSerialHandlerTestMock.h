#include "GSMSerialHandler.h"
#include "SerialStream.h"

class GSMSerialHandlerTestMock: public GSMSerialHandler
{
public: 
	SerialStream * serialStream;

	GSMSerialHandlerTestMock(SerialStream * serialStream);
	~GSMSerialHandlerTestMock();

	void BeginInitialization();
	void FinalizeInitialization();

	void ReadResponse(char * response);

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;
};