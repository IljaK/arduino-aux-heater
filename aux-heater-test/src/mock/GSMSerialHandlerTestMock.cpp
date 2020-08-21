#include "GSMSerialHandlerTestMock.h"

GSMSerialHandlerTestMock::GSMSerialHandlerTestMock(Stream * stream):GSMSerialHandler(NULL, stream), BaseSerialHandlerMock()
{
	
}


GSMSerialHandlerTestMock::~GSMSerialHandlerTestMock()
{
}

void GSMSerialHandlerTestMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	GSMSerialHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}