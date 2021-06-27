#pragma once
#include "Definitions.h"
#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include "common/Util.h"

struct TemperatureData {
	float temperature = 0;
	float humidity = 0;
	float pressure = 0;
	float temperature2 = 0;
};

extern Adafruit_BME280 bme280;
extern OneWire dallasWire;
extern DallasTemperature dallasSensors;

class TemperatureHandler
{
public:
    void Start();
    void GetTemperature(TemperatureData *temperatureData);
};