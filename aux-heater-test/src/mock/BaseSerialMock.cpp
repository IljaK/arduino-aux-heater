#include "BaseSerialMock.h"

BaseSerialMock::BaseSerialMock(SerialStream* serial)
{
	this->serialStream = serial;
}


BaseSerialMock::~BaseSerialMock()
{
}

void BaseSerialMock::HandleResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	isResponseReceived = true;
	isResponseTimeOut = IsTimeOut;
	isResponseOverFlow = isOverFlow;
}

void BaseSerialMock::Clear() {
	isResponseReceived = false;
	isResponseTimeOut = false;
	isResponseOverFlow = false;
}
