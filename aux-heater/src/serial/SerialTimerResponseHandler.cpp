#include "SerialTimerResponseHandler.h"

SerialTimerResponseHandler::SerialTimerResponseHandler(Stream * serial):BaseSerialHandler(serial)
{
}

SerialTimerResponseHandler::~SerialTimerResponseHandler()
{
	StopTimer();
}

void SerialTimerResponseHandler::Loop()
{
	BaseSerialHandler::Loop();
	
	if (registeredBytes < serial->available()) {
		StartTimer();
	}
}

void SerialTimerResponseHandler::OnTimerComplete(TimerID timerId)
{
	if (messageTimer == timerId) {
		messageTimer = 0;
		ResponseDetectedInternal(false, false);
	} else {
		BaseSerialHandler::OnTimerComplete(timerId);
	}
}

void SerialTimerResponseHandler::StartTimer()
{
	StopTimer();
	if (IsLimitReached()) return;
	registeredBytes = serial->available();
	messageTimer = Timer::Start(this, ResponseByteTimeOut());
}
void SerialTimerResponseHandler::StopTimer()
{
	if (messageTimer != 0) {
		Timer::Stop(messageTimer);
		messageTimer = 0;
	}
	StopTimeoutTimer();
}

uint8_t SerialTimerResponseHandler::GetDataBitsAmount()
{
	return 8u;
}
uint8_t SerialTimerResponseHandler::GetStopBitsAmount()
{
	return 1u;
}
uint32_t SerialTimerResponseHandler::GetBaudRate()
{
	return 9600u;
}

unsigned long SerialTimerResponseHandler::ResponseByteTimeOut()
{
	return (unsigned long)(SingleByteTransferDuration() * 2.0);
}

double SerialTimerResponseHandler::SingleByteTransferDuration()
{
	// For baud rate 9600 + 1 start bit + 8 data bit + 1 stop bit
	double bitsPerMessage = 1 + GetDataBitsAmount() + GetStopBitsAmount();
	return ( 1000000.0 / (double)GetBaudRate() * bitsPerMessage);
	//return (((double)bitsPerMessage * 1000.0 / (double)GetBaudRate() * 1000.0));
	//return ((1000.0 / (double)GetBaudRate() * 1000.0));
}

bool SerialTimerResponseHandler::IsBusy()
{
	return messageTimer != 0 || BaseSerialHandler::IsBusy();
}

void SerialTimerResponseHandler::FlushData()
{
	registeredBytes = 0;
	BaseSerialHandler::FlushData();
}

void SerialTimerResponseHandler::ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow)
{
	StopTimer();
	BaseSerialHandler::ResponseDetectedInternal(IsTimeOut, isOverFlow);
}
