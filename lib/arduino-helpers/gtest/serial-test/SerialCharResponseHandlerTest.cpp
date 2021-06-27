#include <gtest/gtest.h>
#include "../mock/SerialCharResponseHandlerMock.h"
#include "../mock/BaseSerialHandlerMock.h"
#include <Arduino.h>
#include <Stream.h>
#include "../mock/TimerMock.h"



TEST(SerialCharResponseHandlerTest, SeparatorParsingTest)
{
	timeOffset = 0;
	TimerMock::Reset();

	char separator[] = "\r\r\r\n";
	Stream serial;
	SerialCharResponseHandlerMock charResponseHandler(128, (const char *)separator, &serial);

	char data1[] = "some args";
	char data2[] = "cmd end\r\r\r\nbegin";
	char data3[] = "cmd test\r\r\r\r\n end\r\r\r\n";

	charResponseHandler.Loop();
	serial.AddRXBuffer(data1);
	charResponseHandler.Loop();

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), 0);

	serial.AddRXBuffer(data2);
	charResponseHandler.Loop();

	int position = strstr(data2, separator) - data2;
	int cmdLength = strlen(data1) + position;

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), cmdLength);

	charResponseHandler.Clear();

	serial.AddRXBuffer(data3);
	charResponseHandler.Loop();

			
	cmdLength = strlen(data2) - position - strlen(separator) + (strstr(data3, separator) - data3);

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), cmdLength);
	charResponseHandler.Loop();
			
	cmdLength = 4;

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), cmdLength);
	charResponseHandler.Clear();
}


TEST(SerialCharResponseHandlerTest, BufferOverflowTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	char data1[] = "some arg"; // length = 8

	char separator[] = "\r\n";
	Stream serial;

    const size_t bufferSize = 128;

	SerialCharResponseHandlerMock charResponseHandler(bufferSize, (const char *)separator, &serial);

	charResponseHandler.Loop();

	int maxIterations = bufferSize / strlen(data1);

	for (int i = 0; i <= maxIterations; i++) {
		serial.AddRXBuffer(data1);
		charResponseHandler.Loop();
		if (strlen(charResponseHandler.receivedCommand) > 0) {

			EXPECT_TRUE(charResponseHandler.isResponseOverFlow);
			EXPECT_EQ(i, maxIterations - 1);
			return;
		}
	}

	FAIL();
}