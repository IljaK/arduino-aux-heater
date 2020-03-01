#include <gtest/gtest.h>
#include "BaseSerialHandlerMock.h"



BaseSerialHandlerMock::BaseSerialHandlerMock()
{
}


BaseSerialHandlerMock::~BaseSerialHandlerMock()
{
}

void BaseSerialHandlerMock::HandleResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	isResponseReceived = true;
	isResponseTimeOut = IsTimeOut;
	isResponseOverFlow = isOverFlow;
}

void BaseSerialHandlerMock::Clear() {
	isResponseReceived = false;
	isResponseTimeOut = false;
	isResponseOverFlow = false;
}
