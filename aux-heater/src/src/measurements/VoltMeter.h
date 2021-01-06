#pragma once
#include "PinValueMeter.h"

class VoltMeter: public PinValueMeter
{
private:

	float r1;
	float r2;
public:
    VoltMeter() {};
	VoltMeter(float r1, float r2, uint8_t measurePin);
	~VoltMeter();

	double Voltage();
	double PinVoltage();

	float R1() { return r1; }
	float R2() { return r2; }
};

