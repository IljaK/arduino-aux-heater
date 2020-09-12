#include <gtest/gtest.h>
#include <Timer.h>
#include <Arduino.h>
#include "mock/TimerMock.h"
TEST(Timer, TimerTestRunStop)
{
	timeOffset = 0;
	TimerMock::Reset();

			
	TimerMock * timerMock = new TimerMock();

	Timer::Loop();
	timerMock->Start(1000000ul);
	Timer::Loop();

	//wchar_t message[128];
	//swprintf(message, 128, L"Timer IsRunning Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());
	//Assert::IsFalse(timerMock->IsCompleted(), message);
	EXPECT_FALSE(timerMock->IsCompleted());

	timeOffset = 1000000ul;
	Timer::Loop();

	EXPECT_TRUE(timerMock->IsCompleted());

	Timer::StopAll(timerMock);

	//swprintf(message, 128, L"Timer Complete Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());
	//Assert::IsTrue(timerMock->IsCompleted(), message);

}

TEST(Timer, TimerTestOverflowMicros)
{
	timeOffset = ULONG_MAX - 500000ul;
	Timer::Loop();

	TimerMock* timerMock = new TimerMock();
	timerMock->Start(1000000ul);
	Timer::Loop();

	timeOffset += 800000ul;

	Timer::Loop();

	EXPECT_FALSE(timerMock->IsCompleted());
	//wchar_t message[128];
	//swprintf(message, 128, L"Timer Overflow IsRunning Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());
	//Assert::IsFalse(timerMock->IsCompleted(), message);

	timeOffset += 200000ul;
	Timer::Loop();

	EXPECT_TRUE(timerMock->IsCompleted());
	//swprintf(message, 128, L"Timer Overflow Complete Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());
	//Assert::IsTrue(timerMock->IsCompleted(), message);


	Timer::StopAll(timerMock);
}

TEST(Timer, TimerFillTest)
{
	timeOffset = 0;
	TimerMock::Reset();

	//int maxTimers = 1 + (rand() % MAXBYTE);
	TimerMock timers[MAXBYTE];
	//wchar_t message[128];

	unsigned long maxVal = 0;

	// Fill timers
	for (int i = 0; i < MAXBYTE; i++) {
		unsigned long random = (unsigned long)rand() + 100ul;
		if (maxVal < random) maxVal = random;
		timers[i].Start(random);
	}

	// Iterate delta time
	unsigned long delta = maxVal / 20;
	for (unsigned long i = 0; i < 19; i++) {
		timeOffset += delta;
		Timer::Loop();
	}

	// Reset completed timers
	for (int i = 0; i < MAXBYTE; i++) {
		if (timers[i].IsCompleted()) {
			timers[i].Start(maxVal);
			EXPECT_EQ(i + 1, timers[i].TimerId());
		}
	}

	// Check proper refill
	for (int i = 0; i < MAXBYTE; i++) {
		for (int n = i+1; n < MAXBYTE; n++) {
			EXPECT_NE(timers[n].TimerId(), timers[i].TimerId());
		}
	}


	timeOffset += maxVal;
	Timer::Loop();

	// Check all completed
	for (int i = 0; i < MAXBYTE; i++) {
		EXPECT_TRUE(timers[i].IsCompleted());
	}

	for (int i = 0; i < MAXBYTE; i++) {
		Timer::StopAll(&timers[i]);
	}
}

TEST(Timer, TimerAddOnCallbackTest)
{
	timeOffset = 0;
	TimerMock::Reset();

	const uint8_t length = 3;
	TimerMock timers[length];

	for (int i = 0; i < length; i++) {
		timers[i].Start(1);
		timers[i].resetOnComplete = true;
	}

	timeOffset += 1;
	Timer::Loop();

	for (int i = 0; i < length; i++) {
		EXPECT_FALSE(timers[i].IsCompleted());
		timers[i].resetOnComplete = false;
	}

	timeOffset += 1;
	Timer::Loop();

	for (int i = 0; i < length; i++) {
		EXPECT_TRUE(timers[i].IsCompleted());
	}
}