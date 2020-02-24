#include "stdafx.h"
#include "CppUnitTest.h"
#include "../aux-heater/Timer.h"
#include <Arduino.h>
#include "TimerMock.h"
#include <cstdlib>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudiControlUnitTest
{		
	TEST_CLASS(TimerTest)
	{
	public:
		
		TEST_METHOD(TimerTestRunStop)
		{
			timeOffset = 0;

			
			TimerMock * timerMock = new TimerMock();

			Timer::Loop();
			timerMock->Start(1000000ul);

			wchar_t message[128];
			swprintf(message, 128, L"Timer IsRunning Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());

			Assert::IsFalse(timerMock->IsCompleted(), message);

			timeOffset = 1000000ul;
			Timer::Loop();

			swprintf(message, 128, L"Timer Complete Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());

			Assert::IsTrue(timerMock->IsCompleted(), message);

		}

		TEST_METHOD(TimerTestOverflowMicros)
		{
			timeOffset = ULONG_MAX - 500000ul;
			Timer::Loop();

			TimerMock* timerMock = new TimerMock();
			timerMock->Start(1000000ul);
			Timer::Loop();

			timeOffset += 800000ul;

			Timer::Loop();

			wchar_t message[128];
			swprintf(message, 128, L"Timer Overflow IsRunning Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());

			Assert::IsFalse(timerMock->IsCompleted(), message);

			timeOffset += 200000ul;
			Timer::Loop();

			swprintf(message, 128, L"Timer Overflow Complete Failed! now: %lu, Remain: %lu", micros(), timerMock->Remain());

			Assert::IsTrue(timerMock->IsCompleted(), message);
		}

		TEST_METHOD(TimerFillTest)
		{
			timeOffset = 0;

			//int maxTimers = 1 + (rand() % MAXBYTE);
			TimerMock timers[MAXBYTE];
			wchar_t message[128];

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
					if (i + 1 != timers[i].TimerId()) {
						swprintf(message, 128, L"Timer has been assigned not ordered ID! i:%d, id:%d", i + 1, timers[i].TimerId());
						Assert::Fail(message);
					}
				}
			}

			// Chech proper refill
			for (int i = 0; i < MAXBYTE; i++) {
				for (int n = i+1; n < MAXBYTE; n++) {
					if (timers[n].TimerId() == timers[i].TimerId()) {
						swprintf(message, 128, L"Duplicate timer ID! %d->%d, %d->%d", n, timers[n].TimerId(), i, timers[i].TimerId());
						Assert::Fail(message);
					}
				}
			}


			timeOffset += maxVal;
			Timer::Loop();

			// Check all completed
			for (int i = 0; i < MAXBYTE; i++) {
				if (!timers[i].IsCompleted()) {
					swprintf(message, 128, L"Timer has not been completed! i:%d, id:%d", i + i, timers[i].TimerId());
					Assert::Fail(message);
				}
			}
		}

	};
}