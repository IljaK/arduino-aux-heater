#include <gtest/gtest.h>
#include "mock/SerialCharResponseHandlerMock.h"
#include "mock/BaseSerialHandlerMock.h"
#include <Arduino.h>
#include "SerialStream.h"
#include <GSMSerialHandler.h>


TEST(GSMSerialHandler, ATResponseTest)
{
	// TODO;
	timeOffset = 0;
	char data1[] = "\r\nA";
	char data2[] = "T\r\r";
	char data3[] = "\nOK\r\n";

	char separator[] = "\r\n";
	SerialStream serial;
	GSMSerialHandler gsmHandler(NULL, &serial);

	gsmHandler.Loop();
	timeOffset = SERIAL_RESPONSE_TIMEOUT;
	gsmHandler.Loop();

	serial.AddRXBuffer(data1);
	gsmHandler.Loop();
	gsmHandler.Loop();
	serial.AddRXBuffer(data2);
	gsmHandler.Loop();
	gsmHandler.Loop();
	serial.AddRXBuffer(data3);
	gsmHandler.Loop();

}