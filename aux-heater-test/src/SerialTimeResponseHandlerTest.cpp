#include <gtest/gtest.h>
#include <Arduino.h>
#include "mock/SerialTimerResponseHandlerMock.h"
#include "mock/BaseSerialHandlerMock.h"
#include "SerialStream.h"
#include "mock/TimerMock.h"

TEST(SerialTimeResponseHandler, BufferOverflowTest)
{
	timeOffset = 0;
	TimerMock::Reset();

	char data1[] = "some arg"; // length = 8

	SerialStream serial;
	SerialTimerResponseHandlerMock timerResponseHandler(&serial);

	timerResponseHandler.Loop();
	//wchar_t message[128];

	int maxIterations = SERIAL_RX_BUFFER_SIZE / strlen(data1) + 1;

	for (int i = 0; i < maxIterations; i++) {
		serial.AddRXBuffer(data1);
		timerResponseHandler.Loop();
		if (timerResponseHandler.receivedLength > 0) {

			EXPECT_TRUE(timerResponseHandler.isResponseOverFlow);
			EXPECT_EQ(i, maxIterations - 2);

			//if (!timerResponseHandler.isResponseOverFlow) {
			//	swprintf(message, 128, L"Command detected before owerflow %d \"%S\"", i, timerResponseHandler.receivedCommand);
			//	Assert::Fail(message);
			//}
			//else if (i != maxIterations - 2) {
			//	swprintf(message, 128, L"Command owerflow happened on wrong index %d \"%S\"", (i + 1), timerResponseHandler.receivedCommand);
			//	Assert::Fail(message);
			//}
			return;
		}
	}

	FAIL();
	//swprintf(message, 128, L"Command owerflow has not been detected\"%S\"", timerResponseHandler.receivedCommand);
	//Assert::Fail(message);
}

TEST(SerialTimeResponseHandler, ResponseTimerTest)
{
	char data1[] = "1"; // length = 8

	timeOffset = 0;
	TimerMock::Reset();

	SerialStream serial;
	SerialTimerResponseHandlerMock timerResponseHandler(&serial);

	Timer::Loop();
	timerResponseHandler.Loop();
	//wchar_t message[128];

	int maxIterations = 2;

	for (int i = 0; i < maxIterations; i++) {
		serial.AddRXBuffer(data1);

		timerResponseHandler.Loop();
		if (timerResponseHandler.isResponseReceived) {

			EXPECT_GE(timerResponseHandler.ResponseByteTimeOut(), micros());
			EXPECT_FALSE(timerResponseHandler.isResponseOverFlow);
			EXPECT_FALSE(timerResponseHandler.isResponseTimeOut);

			/*
			if (timerResponseHandler.ResponseByteTimeOut() < micros()) {
				swprintf(message, 128, L"Command detected befor timer runs out %d \"%S\" %lu < %lu", i, timerResponseHandler.receivedCommand, timerResponseHandler.ResponseByteTimeOut(), micros());
				Assert::Fail(message);
			}

			if (timerResponseHandler.isResponseOverFlow) {
				swprintf(message, 128, L"Command detected as owerflow %d \"%S\"", i, timerResponseHandler.receivedCommand);
				Assert::Fail(message);
			}
			if (timerResponseHandler.isResponseTimeOut) {
				swprintf(message, 128, L"Command detected as time out %d \"%S\"", i, timerResponseHandler.receivedCommand);
				Assert::Fail(message);
			} */

			Timer::StopAll(&timerResponseHandler);
			return;
		}
		timeOffset += timerResponseHandler.ResponseByteTimeOut();
		Timer::Loop();
	}

	FAIL();
	//swprintf(message, 128, L"Command data has not been detected \"%S\"", timerResponseHandler.receivedCommand);
	//Assert::Fail(message);
}