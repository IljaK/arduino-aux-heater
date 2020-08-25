#include <gtest/gtest.h>
#include <Arduino.h>
#include <Timer.h>
#include "SerialStream.h"
#include <AuxHeaterSerial.h>
#include "mock/TimerMock.h"

bool actionCompleted = false;

bool handleHeaterComplete(Stream *stream) {

	actionCompleted = true;

	if (stream->available() > 0) {
		char response[32];
		uint8_t bytes[8];
		uint8_t bytesAmount = stream->readBytes(bytes, 8);

		printBytes(response, 32, bytes, bytesAmount);

		outPrintf("AUX: %s", response);
	}
	return true;
}

TEST(AuxHeaterSerialTest, HandlerTest)
{
	timeOffset = 0;
	TimerMock::Reset();

    SerialStream serial;
    AuxHeaterSerial auxSerialHandler(&serial);

    auxSerialHandler.LaunchHeater(&handleHeaterComplete);
    TimerMock::Loop();

    for (int i = 0; i < AUX_MAX_ATTEMPTS; i++) {
        timeOffset += 2400;
        TimerMock::Loop();

        timeOffset += 162124;
        TimerMock::Loop();
    }

    EXPECT_TRUE(actionCompleted);

}