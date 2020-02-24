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

void TimerMock::OnTimerComplete(TimerID timerId) {
	if (timerId == this->timerId) {
		this->timerId = 0;
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
	timerId = Timer::Start(this, duration);
}
unsigned long TimerMock::Remain()
{
	if (IsCompleted()) return 0;
	return Timer::Remain(timerId);
}