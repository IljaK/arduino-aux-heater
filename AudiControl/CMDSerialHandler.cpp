#include "CMDSerialHandler.h"

CMDSerialHandler::CMDSerialHandler(Stream* stream, StringArrayCallback commandCallback) :SerialTimerResponseHandler(stream)
{
	this->commandCallback = commandCallback;
}

CMDSerialHandler::~CMDSerialHandler()
{
}

void CMDSerialHandler::Loop()
{
	SerialTimerResponseHandler::Loop();
}

void CMDSerialHandler::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	SerialTimerResponseHandler::OnResponseReceived(IsTimeOut);

	int available = serial->available();

	if (available <= 0) return;

	uint8_t commandData[SERIAL_RX_BUFFER_SIZE + 1];
	serial->readBytes(commandData, available);

	if (commandCallback == NULL) return;

	commandData[available] = 0;

	char *arguments[10];
	arguments[0] = (char *)commandData;

	size_t argumentsAmount = SplitString((char *)commandData, (char *)" ", (char **)arguments, (size_t)10);
	commandCallback(arguments, argumentsAmount);
}
