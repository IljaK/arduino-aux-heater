#pragma once
#include "PinValueMeter.h"

class VoltMeter: public PinValueMeter
{
private:
    double divider = 0;
public:
    VoltMeter() {};
	VoltMeter(double divider, uint8_t measurePin);
	~VoltMeter();

	double Voltage();
	double PinVoltage();
};

