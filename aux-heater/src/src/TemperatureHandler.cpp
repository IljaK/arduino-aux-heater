#include "TemperatureHandler.h"

Adafruit_BME280 bme280;
OneWire dallasWire(DS18S20_PIN);
DallasTemperature dallasSensors(&dallasWire);

void TemperatureHandler::Start() {
    bme280.begin(246u);
    dallasSensors.begin();
}

void TemperatureHandler::GetTemperature(TemperatureData* data) {
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage
    if (data == NULL) return;

    data->temperature = bme280.readTemperature();
    data->humidity = bme280.readHumidity();
    data->pressure = bme280.readPressure();

    dallasSensors.requestTemperatures();
    data->temperature2 = dallasSensors.getTempCByIndex(0);
}