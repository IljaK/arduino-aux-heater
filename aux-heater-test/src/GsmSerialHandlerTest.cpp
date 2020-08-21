#include <gtest/gtest.h>
#include "mock/SerialCharResponseHandlerMock.h"
#include "mock/BaseSerialHandlerMock.h"
#include "mock/GSMSerialHandlerTestMock.h"
#include <Arduino.h>
#include "SerialStream.h"
#include "mock/TimerMock.h"


TEST(GSMSerialHandlerTestMock, ATResponseTest)
{
	// TODO;
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	GSMSerialHandlerTestMock gsmHandler(&serial);

	Timer::Loop();
	gsmHandler.Loop();

	timeOffset += SERIAL_RESPONSE_TIMEOUT;
	Timer::Loop();
	gsmHandler.Loop();

	// AT commant launched

}