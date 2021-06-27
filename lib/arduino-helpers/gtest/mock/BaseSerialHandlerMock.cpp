#include <gtest/gtest.h>
#include "BaseSerialHandlerMock.h"

BaseSerialHandlerMock::BaseSerialHandlerMock(Stream *serial):BaseSerialHandler(serial), BaseSerialMock(serial)
{
}

BaseSerialHandlerMock::~BaseSerialHandlerMock()
{
}

void BaseSerialHandlerMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	BaseSerialMock::HandleResponseReceived(IsTimeOut, isOverFlow);
	BaseSerialHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}

