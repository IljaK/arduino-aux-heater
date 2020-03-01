#include <gtest/gtest.h>
#include <Arduino.h>
#include <Timer.h>
#include "BaseSerialHandlerTestMock.h"
#include "SerialStream.h"


TEST(BaseSerialHandler, BufferOverflowTest)
{
	timeOffset = 0;
	char data1[] = "some arg"; // length = 8

	SerialStream stream;
	BaseSerialHandlerTestMock serialHandler(&stream);

	serialHandler.Loop();

	serialHandler.StartTimeoutTimer(1000000ul);
	//wchar_t message[128];

	int maxIterations = SERIAL_RX_BUFFER_SIZE / strlen(data1) + 1;

	for (int i = 0; i < maxIterations; i++) {
		stream.AddRXBuffer(data1);
		serialHandler.Loop();


		if (serialHandler.isResponseReceived) {

			EXPECT_TRUE(serialHandler.isResponseOverFlow);
			EXPECT_EQ(i, maxIterations - 2);

			//if (!serialHandler.isResponseOverFlow) {
				//swprintf(message, 128, L"Command detected before owerflow %d available: %d", i, serialHandler.SerialStream()->available());
				//Assert::Fail(message);
			//}
			//else if (i != maxIterations - 2) {
				//swprintf(message, 128, L"Command owerflow happened on wrong index %d available: %d", (i + 1), serialHandler.SerialStream()->available());
				//Assert::Fail(message);
			//}
			return;
		}
	}

	FAIL();

	//swprintf(message, 128, L"Command owerflow has not been detected available: %d", serialHandler.SerialStream()->available());
	//Assert::Fail(message);
}

TEST(BaseSerialHandler, SerialHandlerTimeoutTest)
{
	timeOffset = 0;

	SerialStream stream;
	BaseSerialHandlerTestMock serialHandler(&stream);
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
}