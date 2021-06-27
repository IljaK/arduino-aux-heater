#include <gtest/gtest.h>
#include "common/Timer.h"
#include <Arduino.h>
#include "measurements/VoltMeter.h"
#include <Util.h>
#include <math.h>
#include "mock/TimerMock.h"



TEST(VoltMeterTest, MeasureValueTest)
{
	timeOffset = 0;
	TimerMock::Reset();

	double expectedVoltage = 12.0;

    double r1 = 20000.0f;
    double r2 = 4700.0f;
    double divider = r2/(r1+r2);

    /*
	VoltMeter voltMeter = VoltMeter(divider, VOLTMETER_MEASURE_PIN);
	analogValues[VOLTMETER_MEASURE_PIN] = voltageToPinValue(voltMeter.R1(), voltMeter.R2(), expectedVoltage, 5.0);
			
	Timer::Loop();

	timeOffset += DELAY_BETWEEN_MEASURE;

	Timer::Loop();

	timeOffset += DELAY_BEFORE_MEASURE;

	Timer::Loop();

	float pinVoltage = voltMeter.PinVoltage();
	float voltage = voltMeter.Voltage();

	EXPECT_GE(voltage, pinVoltage);
	EXPECT_EQ(round(voltage), round(expectedVoltage));

	Timer::StopAll(&voltMeter);
    */

	//wchar_t message[128];
	//if (voltage < pinVoltage) {
	//	swprintf(message, 128, L"Pin voltage higher than result voltage! %.2f > %.2f", pinVoltage, voltage);
	//	Assert::Fail(message);
	//}
	//else if (round(voltage) != round(expectedVoltage)) {
	//	swprintf(message, 128, L"Result voltage far away from expected! %.2f > %.2f", pinVoltage, expectedVoltage);
	//	Assert::Fail(message);
	//}
}