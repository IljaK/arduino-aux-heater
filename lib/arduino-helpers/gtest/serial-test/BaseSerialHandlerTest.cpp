#include <gtest/gtest.h>
#include <Arduino.h>
#include "common/Timer.h"
#include "../mock/BaseSerialHandlerMock.h"
#include <Stream.h>
#include "../mock/TimerMock.h"

TEST(BaseSerialHandlerTest, SerialHandlerTimeoutTest)
{
	timeOffset = 0;
	TimerMock::Reset();

	Stream stream;
	BaseSerialHandlerMock serialHandler(&stream);
	serialHandler.Loop();

	uint8_t aray[] = { 5,5,5 };
	serialHandler.StartTimeoutTimer(1000000ul);

	Timer::Loop();
	serialHandler.Loop();
	EXPECT_FALSE(serialHandler.isResponseReceived);

	timeOffset = 1000000ul;

	Timer::Loop();
	serialHandler.Loop();

	EXPECT_TRUE(serialHandler.isResponseReceived);

	Timer::StopAll(&serialHandler);
}