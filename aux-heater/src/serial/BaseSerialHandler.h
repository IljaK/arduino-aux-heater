#pragma once
#include "../common/Util.h"
#include "../common/Timer.h"
#include <Stream.h>

class BaseSerialHandler: public ITimerCallback
{
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

	virtual bool IsBusy();

	virtual void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false);
	virtual void FlushData();
	virtual void Loop();

	bool IsLimitReached();
};

