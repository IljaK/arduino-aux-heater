#include <gtest/gtest.h>
#include <Arduino.h>
#include <Timer.h>
#include "mock/SerialStream.h"
#include <AuxHeaterSerial.h>
#include "mock/TimerMock.h"

bool actionCompleted = false;

bool handleHeaterComplete(Stream *stream) {

	actionCompleted = true;

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