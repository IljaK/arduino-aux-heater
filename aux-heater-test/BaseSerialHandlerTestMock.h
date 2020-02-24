#pragma once
#include "BaseSerialHandlerMock.h"
#include "../aux-heater/BaseSerialHandler.h"

class BaseSerialHandlerTestMock: public BaseSerialHandlerMock, public BaseSerialHandler
{
public:
	BaseSerialHandlerTestMock(Stream *serial);
	~BaseSerialHandlerTestMock();

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;

	Stream* SerialStream() { return serial; }
};

