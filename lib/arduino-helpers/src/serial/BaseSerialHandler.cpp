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
void BaseSerialHandler::OnTimerStop(TimerID timerId, uint8_t data)
{
	if (responseTimeoutTimer == timerId) {
		responseTimeoutTimer = 0;
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
    // Waits for the transmission of outgoing serial data to complete. 
    // (Prior to Arduino 1.0, this instead removed any buffered incoming serial data.)
	serial->flush();
    while (serial->available()) {
        serial->read();
    }
}

void BaseSerialHandler::Loop()
{
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
