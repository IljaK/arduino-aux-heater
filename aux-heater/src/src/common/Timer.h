#pragma once
#include <Arduino.h>

#if ESP32
typedef uint16_t TimerID;
#else
typedef uint8_t TimerID;
#endif

#define MICRO_SECONDS 1000000ul

class ITimerCallback
{
public:
	virtual ~ITimerCallback();
	virtual void OnTimerComplete(TimerID timerId, uint8_t data);
	virtual void OnTimerStop(TimerID timerId, uint8_t data) {};
};

typedef void (*timerCompleteCallBack) (TimerID timerId, uint8_t data);

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
#if ESP32
	static SemaphoreHandle_t xTimerSemaphore;
#endif
	static TimerNode *pFirst;
protected:
	static unsigned long frameTS;
public:
	static TimerID Start(ITimerCallback *pCaller, unsigned long duration, uint8_t data = 0);
	static TimerID Start(timerCompleteCallBack pCallBack, unsigned long duration, uint8_t data = 0);
	static void Loop();
	static bool Stop(TimerID timerId);
	static bool Contains(ITimerCallback *pCaller, uint8_t data);
	static unsigned long Remain(TimerID timerId);
	static void StopAll(ITimerCallback* pCaller);
    static uint8_t GetData(TimerID timerId);

    class TimerCallback: public ITimerCallback
    {
    private:
        timerCompleteCallBack cb = NULL;
    public:
        TimerCallback(timerCompleteCallBack cb) {
            this->cb = cb;
        };
        ~TimerCallback() { };
        void OnTimerComplete(TimerID timerId, uint8_t data) override {
            this->cb(timerId, data);
            delete this;
        }
        void OnTimerStop(TimerID timerId, uint8_t data) override {
            this->cb(timerId, data);
            delete this;
        }
    };
};
