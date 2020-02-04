#include "Timer.h"

Timer::Timer()
{
	isRunning = false;
}

Timer::~Timer()
{
}

unsigned long Timer::CompleteTS()
{
	return (startTS + duration);
}

void Timer::Loop()
{
	if (!isRunning) return;

	unsigned long completeTs = CompleteTS();
	unsigned long micrs = micros();

	if (Remain() == 0) {
		Stop();
	}
}

// Duration in micro seconds
void Timer::Start(unsigned long duration)
{
	startTS = micros();
	this->duration = duration;
	isRunning = true;
}
void Timer::Stop()
{
	isRunning = false;
}

bool Timer::IsRunning()
{
	return isRunning;
}

void Timer::Reset()
{
	if (duration > 0) {
		Stop();
		Start(duration);
	}
}

unsigned long Timer::StartTS()
{ 
	return startTS; 
}
unsigned long Timer::Duration() 
{ 
	return duration;
}
unsigned long Timer::Remain()
{
	if (!isRunning) return 0;

	unsigned long completeTs = CompleteTS();
	unsigned long micrs = micros();


	if (completeTs == startTS) {
		return 0;
	}
	if (completeTs > startTS) {
		if (micrs < startTS || micrs > completeTs) return 0;
		//return completeTs - micrs - startTS;
	}
	else {
		if (micrs < startTS && micrs >= completeTs) return 0;
	}

	return completeTs - micrs - startTS;
}
