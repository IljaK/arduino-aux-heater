#pragma once
#include "Arduino.h"

class Adafruit_BME280 {
public:
    void begin(uint8_t) {}
    float readTemperature() { return 0;}
    float readHumidity() { return 0;}
    float readPressure() { return 0;}
};