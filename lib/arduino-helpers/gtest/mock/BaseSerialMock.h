#pragma once
#include "serial/BaseSerialHandler.h"
#include <Stream.h>

class BaseSerialMock
{
public:
	Stream* serialStream;

	bool isResponseReceived = false;
	bool isResponseTimeOut = false;
	bool isResponseOverFlow = false;

	BaseSerialMock(Stream* serial);
	~BaseSerialMock();

	void HandleResponseReceived(bool IsTimeOut, bool isOverFlow);

	virtual void Clear();
};

