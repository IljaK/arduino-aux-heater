#pragma once
#include <Arduino.h>

typedef uint8_t TimerID;

class ITimerCallback
{
public:
	virtual ~ITimerCallback();
	virtual void OnTimerComplete(TimerID timerId);
};

struct TimerNode
{
	ITimerCallback *pCaller = NULL;
	TimerID id = 0;
	unsigned long remain = 0;
	TimerNode *pNext = NULL;
};

class Timer
{
private:
	static TimerNode *pFirst;
	static unsigned long frameTS;
public:
	static TimerID Start(ITimerCallback *pCaller, unsigned long duration);
	static void Loop();
	static bool Stop(TimerID timerId);
	static unsigned long Remain(TimerID timerId);
	static bool StopAll(ITimerCallback* pCaller);
};
