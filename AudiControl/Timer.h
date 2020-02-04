#pragma once
#include <Arduino.h>

class Timer
{
protected:
	unsigned long startTS;
	unsigned long duration;
	bool isRunning;

public:
	Timer();
	~Timer();

	virtual void Loop();
	void Reset();
	virtual void Start(unsigned long duration);
	virtual void Stop();
	virtual bool IsRunning();
	unsigned long CompleteTS();

	unsigned long StartTS();
	unsigned long Duration();
	unsigned long Remain();

};

