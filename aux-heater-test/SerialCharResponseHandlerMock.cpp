#include "SerialCharResponseHandlerMock.h"



SerialCharResponseHandlerMock::SerialCharResponseHandlerMock(const char *separator, Stream * stream):SerialCharResponseHandler(separator, stream), BaseSerialHandlerMock()
{
	Clear();
}


SerialCharResponseHandlerMock::~SerialCharResponseHandlerMock()
{
}

void SerialCharResponseHandlerMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	HandleResponseReceived(IsTimeOut, isOverFlow);
	SerialCharResponseHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}

void SerialCharResponseHandlerMock::HandleResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	if (bufferLength > 0) {
		strcpy(receivedCommand, buffer);
	}
	BaseSerialHandlerMock::HandleResponseReceived(IsTimeOut, isOverFlow);
}

void SerialCharResponseHandlerMock::Clear() {
	receivedCommand[0] = 0;
	BaseSerialHandlerMock::Clear();
}