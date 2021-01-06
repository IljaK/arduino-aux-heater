#pragma once
#include "PinValueMeter.h"

class AmperMeter: public PinValueMeter
{
public:
    AmperMeter() {} ;
    AmperMeter(uint8_t measurePin);
    double Ampers();
};