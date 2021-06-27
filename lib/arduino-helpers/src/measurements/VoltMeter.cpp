#include "VoltMeter.h"
#include "../common/Util.h"

VoltMeter::VoltMeter(double divider, uint8_t measurePin):PinValueMeter(measurePin)
{
    this->divider = divider;
    //divider = (double)r2 / ((double)r1 + (double)r2);

	pinMode(measurePin, INPUT);
    Measure();
}


VoltMeter::~VoltMeter()
{

}

double VoltMeter::PinVoltage()
{
	return pinValue;
}

double VoltMeter::Voltage()
{
    if (pinValue == 0) {
        return 0;
    }

	double voltage = (double)pinValue / divider;

	if (voltage < 0.1) voltage = 0.0;

	return voltage;
}

