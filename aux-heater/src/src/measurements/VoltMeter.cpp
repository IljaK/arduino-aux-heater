#include "VoltMeter.h"
#include "../common/Util.h"

VoltMeter::VoltMeter(float r1, float r2, uint8_t measurePin):PinValueMeter(measurePin)
{
	this->r1 = r1;
	this->r2 = r2;

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

	double voltage = (double)pinValue / ((double)r2 / ((double)r1 + (double)r2));

	if (voltage < 0.1) voltage = 0.0;

	return voltage;
}

