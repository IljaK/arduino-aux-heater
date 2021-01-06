#include "PinValueMeter.h"

PinValueMeter::PinValueMeter(uint8_t measurePin)
{
    this->measurePin = measurePin;
}
void PinValueMeter::Measure()
{
    if (measurePin > 0) {
        pinValue = readAnalogVoltage(measurePin);
    }
}