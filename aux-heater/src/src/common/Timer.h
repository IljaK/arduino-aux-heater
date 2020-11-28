#pragma once
#include <Arduino.h>

#ifdef ESP32
typedef uint16_t TimerID;
#else
typedef uint8_t TimerID;
#endif

class ITimerCallback
{
public:
	virtual ~ITimerCallback();
	virtual void OnTimerComplete(TimerID timerId, uint8_t data);
};

struct TimerNode
{
	ITimerCallback *pCaller = NULL;
	TimerID id = 0;
	unsigned long remain = 0;
	uint8_t data = 0;
	TimerNode *pNext = NULL;
};

class Timer
{
private:
	static SemaphoreHandle_t xTimerSemaphore;
	static TimerNode *pFirst;
protected:
	static unsigned long frameTS;
public:
	static TimerID Start(ITimerCallback *pCaller, unsigned long duration, uint8_t data = 0);
	static void Loop();
	static bool Stop(TimerID timerId);
	static bool Contains(ITimerCallback *pCaller, uint8_t data);
	static unsigned long Remain(TimerID timerId);
	static void StopAll(ITimerCallback* pCaller);
};
