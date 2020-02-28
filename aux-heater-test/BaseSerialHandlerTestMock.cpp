#include "gtest/gtest.h"
#include "BaseSerialHandlerTestMock.h"



BaseSerialHandlerTestMock::BaseSerialHandlerTestMock(Stream *serial):BaseSerialHandlerMock(), BaseSerialHandler(serial)
{
}


BaseSerialHandlerTestMock::~BaseSerialHandlerTestMock()
{
}

void BaseSerialHandlerTestMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	BaseSerialHandlerMock::HandleResponseReceived(IsTimeOut, isOverFlow);
	BaseSerialHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}
