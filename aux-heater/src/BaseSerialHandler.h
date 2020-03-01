#pragma once
#include "Util.h"
#include "Timer.h"
#include <Stream.h>

class BaseSerialHandler: public ITimerCallback
{
private:
	void (*responseCallback)() = NULL;
protected:
	Stream * serial = NULL;
	TimerID responseTimeoutTimer = 0;

	virtual void ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow = false);
	
public:
	BaseSerialHandler(Stream * serial);
	~BaseSerialHandler();

	void StartTimeoutTimer(unsigned long microSecTimeOut);
	void StopTimeoutTimer();
	void OnTimerComplete(TimerID timerId) override;

	//size_t WriteData(char* byteArray, uint8_t length, void (*callback)() = NULL, long microSecTimeOut = SERIAL_RESPONSE_TIMEOUT);
	//virtual size_t WriteData(uint8_t* byteArray, uint8_t length, void (*responseCallback)() = NULL, unsigned long microSecTimeOut = (unsigned long)SERIAL_RESPONSE_TIMEOUT);
	virtual bool IsBusy();

	virtual void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false);
	virtual void FlushData();
	virtual void Loop();

	bool IsLimitReached();
};

