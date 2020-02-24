#pragma once
#include <Arduino.h>
#include "Util.h"
#include "Timer.h"
#include "BitDataManager.h"

class DigitalOscilloscope: public ITimerCallback
{
public:
	DigitalOscilloscope(uint8_t signalPin);
	~DigitalOscilloscope();

	void OnTimerComplete(TimerID timerId) override;
	void Loop();
	uint8_t SignalPin();

private:
	BitDataManager bitDataManager;
	TimerID timeoutTimer = 0;
	uint8_t actualSignal = HIGH;
	uint8_t signalPin = 0;

	unsigned long lastArrayCompleteTS = 0;

	void ReadLoop(uint8_t pinValue, long microsTS);

	void PrintTable();
	void PrintBitRow(BitRow *bitRow, BitRow * nextBitRow);
};

