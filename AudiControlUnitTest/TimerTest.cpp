#include "stdafx.h"
#include "CppUnitTest.h"
#include "../AudiControl/Timer.h"
#include <Arduino.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudiControlUnitTest
{		
	TEST_CLASS(TimerTest)
	{
	public:
		
		TEST_METHOD(TimerTestRunStop)
		{
			Timer timer;
			timeOffset = 0;

			timer.Start(1000000);
			timer.Loop();

			wchar_t message[128];
			swprintf(message, 128, L"Timer IsRunning Failed! now: %lu, StartTS: %lu, CompleteTS: %lu, Duration:%lu", micros(), timer.StartTS(), timer.CompleteTS(), timer.Duration());

			Assert::IsTrue(timer.IsRunning(), message);

			timeOffset = 1000000;
			timer.Loop();

			swprintf(message, 128, L"Timer Complete Failed! now: %lu, StartTS: %lu, CompleteTS: %lu, Duration:%lu", micros(), timer.StartTS(), timer.CompleteTS(), timer.Duration());

			Assert::IsTrue(!timer.IsRunning(), message);

		}

		TEST_METHOD(TimerTestOverflowMicros)
		{
			Timer timer;

			timeOffset = ULONG_MAX - 500000ul;

			timer.Start(1000000);
			timer.Loop();

			timeOffset += 800000;

			timer.Loop();

			wchar_t message[128];
			swprintf(message, 128, L"Timer Overflow IsRunning Failed! now: %lu, StartTS: %lu, CompleteTS: %lu, Duration:%lu", micros(), timer.StartTS(), timer.CompleteTS(), timer.Duration());

			Assert::IsTrue(timer.IsRunning(), message);

			timeOffset += 200000;
			timer.Loop();

			swprintf(message, 128, L"Timer Overflow Complete Failed! now: %lu, StartTS: %lu, CompleteTS: %lu, Duration:%lu", micros(), timer.StartTS(), timer.CompleteTS(), timer.Duration());

			Assert::IsTrue(!timer.IsRunning(), message);
		}

	};
}