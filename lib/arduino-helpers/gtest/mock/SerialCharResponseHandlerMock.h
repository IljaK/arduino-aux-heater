#pragma once
#include "serial/SerialCharResponseHandler.h"
#include <Stream.h>
#include "BaseSerialHandlerMock.h"

class SerialCharResponseHandlerMock: public SerialCharResponseHandler, public BaseSerialMock
{
public:
	char receivedCommand[SERIAL_RX_BUFFER_SIZE];

	SerialCharResponseHandlerMock(const size_t bufferSize, const char *separator, Stream* stream);
	~SerialCharResponseHandlerMock();

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;
	void Clear() override;
};
