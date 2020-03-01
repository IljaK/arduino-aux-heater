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
	//wchar_t message[128];

	charResponseHandler.Loop();
	serial.AddRXBuffer(data1);
	charResponseHandler.Loop();

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), 0);

	//if (strlen(charResponseHandler.receivedCommand) > 0) {
		//swprintf(message, 128, L"Detected command without proper separator parse \"%S\"", charResponseHandler.receivedCommand);
		//Assert::Fail(message);
		//return;
	//}

	serial.AddRXBuffer(data2);
	charResponseHandler.Loop();

	int position = strstr(data2, separator) - data2;
	int cmdLength = strlen(data1) + position;

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), cmdLength);
	//if (strlen(charResponseHandler.receivedCommand) != cmdLength) {
	//	swprintf(message, 128, L"Result command does not match length (0): %d \"%S\"", cmdLength, charResponseHandler.receivedCommand);
	//	Assert::Fail(message);
	//	return;
	//}

	charResponseHandler.Clear();

	serial.AddRXBuffer(data3);
	charResponseHandler.Loop();

			
	cmdLength = strlen(data2) - position - strlen(separator) + (strstr(data3, separator) - data3);

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), cmdLength);
	//if (strlen(charResponseHandler.receivedCommand) != cmdLength) {
	//	swprintf(message, 128, L"Result command does not match length (1): %d \"%S\"", cmdLength, charResponseHandler.receivedCommand);
	//	Assert::Fail(message);
	//}
	charResponseHandler.Loop();
			
	cmdLength = 4;

	EXPECT_EQ(strlen(charResponseHandler.receivedCommand), cmdLength);
	//if (strlen(charResponseHandler.receivedCommand) != cmdLength) {
	//	swprintf(message, 128, L"Result command does not match length (2): %d \"%S\"", cmdLength, charResponseHandler.receivedCommand);
	//	Assert::Fail(message);
	//}
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
	wchar_t message[128];

	int maxIterations = SERIAL_RX_BUFFER_SIZE / strlen(data1) + 1;

	for (int i = 0; i < maxIterations; i++) {
		serial.AddRXBuffer(data1);
		charResponseHandler.Loop();
		if (strlen(charResponseHandler.receivedCommand) > 0) {

			EXPECT_TRUE(charResponseHandler.isResponseOverFlow);
			EXPECT_EQ(i, maxIterations - 2);

			//if (!charResponseHandler.isResponseOverFlow) {
			//	swprintf(message, 128, L"Command detected before owerflow %d \"%S\"", i, charResponseHandler.receivedCommand);
			//	Assert::Fail(message);
			//} else if ( i != maxIterations - 2) {
			//	swprintf(message, 128, L"Command owerflow happened on wrong index %d \"%S\"", (i + 1), charResponseHandler.receivedCommand);
			//	Assert::Fail(message);
			//}
			return;
		}
	}

	FAIL();

	//swprintf(message, 128, L"Command owerflow has not been detected\"%S\"", charResponseHandler.receivedCommand);
	//Assert::Fail(message);
}