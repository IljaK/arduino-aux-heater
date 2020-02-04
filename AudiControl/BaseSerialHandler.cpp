#include "BaseSerialHandler.h"

BaseSerialHandler::BaseSerialHandler(Stream * serial)
{
	this->serial = serial;
}

BaseSerialHandler::~BaseSerialHandler()
{

}


void BaseSerialHandler::ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow)
{
	StopTimeoutTimer();
	OnResponseReceived(IsTimeOut, isOverFlow);
}

void BaseSerialHandler::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	if (responseCallback != NULL)
	{
		void (*cb)() = responseCallback;
		responseCallback = NULL;
		cb();
	}
}

size_t BaseSerialHandler::WriteData(uint8_t* byteArray, uint8_t length, void(*responseCallback)(), unsigned long microSecTimeOut)
{
	this->responseCallback = responseCallback;
	size_t requestLength = serial->write(byteArray, length);
	StartTimeoutTimer(microSecTimeOut);
	return requestLength;
}

Stream * BaseSerialHandler::SerialStream()
{
	return serial;
}

bool BaseSerialHandler::IsBusy()
{
	return responseTimeoutTimer.IsRunning();
}

void BaseSerialHandler::FlushData()
{
	serial->flush();
}

void BaseSerialHandler::Loop()
{
	bool wasLaunchedRT = responseTimeoutTimer.IsRunning();

	responseTimeoutTimer.Loop();

	if (wasLaunchedRT) {
		if (!responseTimeoutTimer.IsRunning()) {
			ResponseDetectedInternal(true, false);
			return;
		}
		if (serial->available() >= SERIAL_RX_BUFFER_SIZE) {
			ResponseDetectedInternal(false, true);
			return;
		}
	}
}

void BaseSerialHandler::StartTimeoutTimer(unsigned long microSecTimeOut)
{
	if (microSecTimeOut == 0) return;
	responseTimeoutTimer.Start(microSecTimeOut);
}
void BaseSerialHandler::StopTimeoutTimer()
{
	responseTimeoutTimer.Stop();
}
