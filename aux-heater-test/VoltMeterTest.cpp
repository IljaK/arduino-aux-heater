#include "stdafx.h"
#include "CppUnitTest.h"
#include "../aux-heater/Timer.h"
#include <Arduino.h>
#include "../aux-heater/VoltMeter.h"
#include "../aux-heater/Util.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudiControlUnitTest
{
	TEST_CLASS(VoltMeterTest)
	{
	public:

		TEST_METHOD(MeasureValueTest)
		{
			timeOffset = 0;
			double expectedVoltage = 12.0;

			VoltMeter voltMeter = VoltMeter(20000.0f, 4700.0f);
			analogValues[VOLTMETER_MEASURE_PIN] = voltageToPinValue(voltMeter.R1(), voltMeter.R2(), expectedVoltage, 5.0);
			
			Timer::Loop();

			timeOffset += DELAY_BETWEEN_MEASURE;

			Timer::Loop();

			timeOffset += DELAY_BEFORE_MEASURE;

			Timer::Loop();

			float pinVoltage = voltMeter.PinVoltage();
			float voltage = voltMeter.Voltage();

			wchar_t message[128];
			if (voltage < pinVoltage) {
				swprintf(message, 128, L"Pin voltage higher than result voltage! %.2f > %.2f", pinVoltage, voltage);
				Assert::Fail(message);
			}
			else if (round(voltage) != round(expectedVoltage)) {
				swprintf(message, 128, L"Result voltage far away from expected! %.2f > %.2f", pinVoltage, expectedVoltage);
				Assert::Fail(message);
			}
		}

	};
}