#include "BaseSerialHandler.h"

BaseSerialHandler::BaseSerialHandler(Stream * serial):ITimerCallback()
{
	this->serial = serial;
}

BaseSerialHandler::~BaseSerialHandler()
{
	StopTimeoutTimer();
}

void BaseSerialHandler::OnTimerComplete(TimerID timerId, uint8_t data)
{
	if (responseTimeoutTimer == timerId) {
		responseTimeoutTimer = 0;
		ResponseDetectedInternal(true, false);
	}
}


void BaseSerialHandler::ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow)
{
	StopTimeoutTimer();
	OnResponseReceived(IsTimeOut, isOverFlow);
}

void BaseSerialHandler::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
}

bool BaseSerialHandler::IsBusy()
{
	return responseTimeoutTimer != 0;
}

void BaseSerialHandler::FlushData()
{
	serial->flush();
}

void BaseSerialHandler::Loop()
{
	if (IsLimitReached()) {
		ResponseDetectedInternal(false, true);
	}
}

bool BaseSerialHandler::IsLimitReached()
{
	return (serial != NULL && serial->available() >= SERIAL_CHAR_BUFFER_SIZE);
}

void BaseSerialHandler::StartTimeoutTimer(unsigned long microSecTimeOut)
{
	if (microSecTimeOut == 0) return;
	StopTimeoutTimer();
	responseTimeoutTimer = Timer::Start(this, microSecTimeOut);
}
void BaseSerialHandler::StopTimeoutTimer()
{
	if (responseTimeoutTimer != 0) Timer::Stop(responseTimeoutTimer);
	responseTimeoutTimer = 0;
}
