#include "SerialTimerResponseHandlerMock.h"

SerialTimerResponseHandlerMock::SerialTimerResponseHandlerMock(SerialStream *serial): SerialTimerResponseHandler(serial), BaseSerialMock(serial)
{
	receivedCommand[0] = 0;
	receivedLength = 0;
}

SerialTimerResponseHandlerMock::~SerialTimerResponseHandlerMock()
{
}

void SerialTimerResponseHandlerMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	if (registeredBytes > 0) {
		serialStream->readBytes(receivedCommand, registeredBytes);
	}
	receivedLength = registeredBytes;
	BaseSerialMock::HandleResponseReceived(IsTimeOut, isOverFlow);
	SerialTimerResponseHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}

void SerialTimerResponseHandlerMock::Clear() {
	receivedCommand[0] = 0;
	BaseSerialMock::Clear();
}