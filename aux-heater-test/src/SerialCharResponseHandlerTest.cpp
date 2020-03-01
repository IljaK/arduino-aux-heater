#include <gtest/gtest.h>
#include "SerialCharResponseHandlerMock.h"
#include "BaseSerialHandlerMock.h"
#include <Arduino.h>
#include "SerialStream.h"



TEST(SerialCharResponseHandler, SeparatorParsingTest)
{
	timeOffset = 0;
	char separator[] = "\r\r\r\n";
	SerialStream serial;
	SerialCharResponseHandlerMock charResponseHandler((const char *)separator, &serial);

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


TEST(SerialCharResponseHandler, BufferOverflowTest)
{
	timeOffset = 0;
	char data1[] = "some arg"; // length = 8

	char separator[] = "\r\n";
	SerialStream serial;
	SerialCharResponseHandlerMock charResponseHandler((const char *)separator, &serial);

	charResponseHandler.Loop();

	int maxIterations = SERIAL_RX_BUFFER_SIZE / strlen(data1) + 1;

	for (int i = 0; i < maxIterations; i++) {
		serial.AddRXBuffer(data1);
		charResponseHandler.Loop();
		if (strlen(charResponseHandler.receivedCommand) > 0) {

			EXPECT_TRUE(charResponseHandler.isResponseOverFlow);
			EXPECT_EQ(i, maxIterations - 2);
			return;
		}
	}

	FAIL();
}