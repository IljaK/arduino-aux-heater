#pragma once
#include "../AudiControl/BaseSerialHandler.h"

class BaseSerialHandlerMock
{
public:
	bool isResponseReceived = false;
	bool isResponseTimeOut = false;
	bool isResponseOverFlow = false;

	virtual void HandleResponseReceived(bool IsTimeOut, bool isOverFlow);
	BaseSerialHandlerMock();
	~BaseSerialHandlerMock();

	virtual void Clear();
};

