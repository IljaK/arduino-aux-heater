#pragma once
#include "Arduino.h"
#include "../common/Util.h"

class PinValueMeter
{
private:
    uint8_t measurePin = 0;
protected:
	double pinValue = 0;
public:
    PinValueMeter() {};
    PinValueMeter(uint8_t measurePin);
    virtual void Measure();
};