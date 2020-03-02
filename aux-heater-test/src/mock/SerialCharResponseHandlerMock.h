#pragma once
#include <SerialCharResponseHandler.h>
#include <Stream.h>
#include "BaseSerialHandlerMock.h"

class SerialCharResponseHandlerMock: public SerialCharResponseHandler, public BaseSerialHandlerMock
{
public:
	char receivedCommand[SERIAL_RX_BUFFER_SIZE];

	SerialCharResponseHandlerMock(const char *separator, Stream * stream);
	~SerialCharResponseHandlerMock();

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;
	void HandleResponseReceived(bool IsTimeOut, bool isOverFlow) override;
	void Clear() override;
};

