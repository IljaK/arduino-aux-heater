#pragma once
#include "../aux-heater/Timer.h"

class TimerMock: public ITimerCallback
{
private:
	TimerID timerId = 0;
public:
	TimerMock();
	~TimerMock();
	void OnTimerComplete(TimerID timerId) override;
	bool IsCompleted();
	void Start(unsigned long duration);
	uint8_t TimerId() { return timerId; }
	unsigned long Remain();
};

