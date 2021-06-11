#include "SerialCharResponseHandlerMock.h"

SerialCharResponseHandlerMock::SerialCharResponseHandlerMock(const size_t bufferSize, const char *separator, SerialStream * stream):SerialCharResponseHandler(bufferSize, separator, stream), BaseSerialMock(stream)
{
	Clear();
}


SerialCharResponseHandlerMock::~SerialCharResponseHandlerMock()
{
}

void SerialCharResponseHandlerMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	if (bufferLength > 0) {
		strcpy(receivedCommand, buffer);
	}
	HandleResponseReceived(IsTimeOut, isOverFlow);
	SerialCharResponseHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}

void SerialCharResponseHandlerMock::Clear() {
	receivedCommand[0] = 0;
	BaseSerialMock::Clear();
}