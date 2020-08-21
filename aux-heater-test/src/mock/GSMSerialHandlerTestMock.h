#include "GSMSerialHandler.h"
#include "BaseSerialHandlerMock.h"

class GSMSerialHandlerTestMock: public GSMSerialHandler, public BaseSerialHandlerMock
{
public: 
	GSMSerialHandlerTestMock(Stream * serial);
	~GSMSerialHandlerTestMock();

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;
};