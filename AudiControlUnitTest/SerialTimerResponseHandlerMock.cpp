#include "SerialTimerResponseHandlerMock.h"



SerialTimerResponseHandlerMock::SerialTimerResponseHandlerMock(Stream *serial): SerialTimerResponseHandler(serial), BaseSerialHandlerMock()
{
}

SerialTimerResponseHandlerMock::~SerialTimerResponseHandlerMock()
{
}

void SerialTimerResponseHandlerMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	HandleResponseReceived(IsTimeOut, isOverFlow);
	SerialTimerResponseHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}

void SerialTimerResponseHandlerMock::HandleResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	if (registeredBytes > 0) {
		serial->readBytes(receivedCommand, registeredBytes);
	}
	receivedLength = registeredBytes;
	BaseSerialHandlerMock::HandleResponseReceived(IsTimeOut, isOverFlow);
}

void SerialTimerResponseHandlerMock::Clear() {
	receivedCommand[0] = 0;
	BaseSerialHandlerMock::Clear();
}