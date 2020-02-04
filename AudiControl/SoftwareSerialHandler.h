#pragma once
#include "Util.h"
#include <SoftwareSerial.h>

class SoftwareSerialHandler: public SoftwareSerial
{
protected:
	uint32_t serialBaudRate;
	// Baud rates: 150 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 31250, 38400, 57600, and 115200.
	// RX - Read
	// TX - Transfer

	//CustomSoftwareSerial * digitalSerial; // RX, TX

	void StartDeviceSerial(uint32_t baudRate);

public:
	SoftwareSerialHandler(uint8_t rxPin, uint8_t txPin);
	~SoftwareSerialHandler();

	uint8_t GetDataBitsAmount();
	uint8_t GetStopBitsAmount();
	uint32_t GetBaudRate();

	virtual void SwitchSerialConfig(char *serialConfig);
	virtual void SwitchBaudRate(char *serialBaudRate);

	void PrintSerialConfig();
};

