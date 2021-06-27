#include "AmperMeter.h"

AmperMeter::AmperMeter(uint8_t measurePin):PinValueMeter(measurePin)
{

}

double AmperMeter::Ampers()
{
    // +/- 10A
    // 2.5V +/- 0.625v
    // (pinValue - 2.5) / 0.625 * 10;
    return (pinValue - 2.5) / 6.25;
}