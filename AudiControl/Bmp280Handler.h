#pragma once

#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include "Util.h"

class Bmp280Handler
{
public:

	// Fro debug
	void PrintData();
	void Setup();
	void Loop();

	Bmp280Handler();
	~Bmp280Handler();
};

