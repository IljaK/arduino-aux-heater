#pragma once
#include <Timer.h>

class TimerMock: public Timer, public ITimerCallback
{
private:
	TimerID timerId = 0;
public:
	unsigned long duration = 0;
	bool resetOnComplete = false;

	TimerMock();
	~TimerMock();
	void OnTimerComplete(TimerID timerId) override;
	bool IsCompleted();
	void Start(unsigned long duration);
	uint8_t TimerId() { return timerId; }
	unsigned long Remain();

	static void Reset();
};

