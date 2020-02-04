#include "Bmp280Handler.h"

Adafruit_BMP280 bmp280;

void Bmp280Handler::PrintData()
{
	Serial.print(F("Temperature = "));
	Serial.print(bmp280.readTemperature());
	Serial.println(" *C");

	Serial.print(F("Pressure = "));
	Serial.print(bmp280.readPressure());
	Serial.println(" Pa");

	Serial.print(F("Approx altitude = "));
	Serial.print(bmp280.readAltitude(1013.25)); /* Adjusted to local forecast! */
	Serial.println(" m");

	Serial.println();
}

void Bmp280Handler::Setup()
{
	while (!bmp280.begin(BMP280_ADDRESS_ALT)) {
		SerialPrintf("Could not find i2c bmp280!");
		delay(1000);
	}

	SerialPrintf("i2c bmp280 started!");
	/*
	Wire.begin();

	for (uint8_t address = 8; address < 127; address++) {

		Wire.beginTransmission(address);

		if (Wire.endTransmission() == 0) {
			Serial.print("I2C device found at address 0x");
			Serial.print(address, HEX);
			break;
		}
	}
	*/
}

void Bmp280Handler::Loop()
{
}

Bmp280Handler::Bmp280Handler()
{
}


Bmp280Handler::~Bmp280Handler()
{
}
