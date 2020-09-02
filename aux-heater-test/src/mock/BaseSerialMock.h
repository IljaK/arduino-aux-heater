#pragma once
#include <BaseSerialHandler.h>
#include "SerialStream.h"

class BaseSerialMock
{
public:
	SerialStream* serialStream;

	bool isResponseReceived = false;
	bool isResponseTimeOut = false;
	bool isResponseOverFlow = false;

	BaseSerialMock(SerialStream* serial);
	~BaseSerialMock();

	void HandleResponseReceived(bool IsTimeOut, bool isOverFlow);

	virtual void Clear();
};

