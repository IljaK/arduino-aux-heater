#include "TimerMock.h"

TimerMock::TimerMock()
{
}

TimerMock::~TimerMock()
{
	if (timerId != 0) {
		Timer::Stop(timerId);
		timerId = 0;
	}
}

void TimerMock::OnTimerComplete(TimerID timerId, uint8_t data) {
	if (timerId == this->timerId) {
		this->timerId = 0;
		if (resetOnComplete) {
			this->timerId = Timer::Start(this, duration, data);
		}
	}
}
bool TimerMock::IsCompleted() {
	return timerId == 0;
}
void TimerMock::Start(unsigned long duration) {
	if (timerId != 0) {
		Timer::Stop(timerId);
		timerId = 0;
	}
	this->duration = duration;
	timerId = Timer::Start(this, duration);
}
unsigned long TimerMock::Remain()
{
	if (IsCompleted()) return 0;
	return Timer::Remain(timerId);
}
void TimerMock::Reset()
{
	frameTS = 0;
}