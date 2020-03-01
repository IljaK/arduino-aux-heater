#pragma once
#include <SerialTimerResponseHandler.h>
#include "BaseSerialHandlerMock.h"

class SerialTimerResponseHandlerMock: public SerialTimerResponseHandler, public BaseSerialHandlerMock
{
public:
	uint8_t receivedCommand[SERIAL_RX_BUFFER_SIZE];
	uint8_t receivedLength = 0;

	SerialTimerResponseHandlerMock(Stream *serial);
	~SerialTimerResponseHandlerMock();

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;
	void HandleResponseReceived(bool IsTimeOut, bool isOverFlow) override;
	void Clear() override;
};

