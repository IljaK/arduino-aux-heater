#pragma once
#include "Timer.h"

struct LedBlinkFrequency
{
private:
	// First 3 bits will store tact Length + 1 bit led state
	uint16_t data = 0;

public:

	uint8_t GetTactLength() {
		// Get rid of first bit
		return (uint8_t)(data >> (DataBytes() - (uint8_t)4));

	}
	void SetTactLength(uint8_t value) {
		uint8_t max = MaxLength();

		if (value > max) value = max;

		uint16_t mask = data & 0x0FFF;
		uint8_t shiftAmount = DataBytes() - (uint8_t)4;
		data = (uint16_t)((value << shiftAmount) | mask);
	}

	bool GetBitValue(uint16_t index) {
		return (data & ((uint16_t)1 << (uint16_t)index)) != 0;
	}

	void SetBitValue(uint16_t index, bool value) {
		if (value) data = data | (1 << index);
		else data = data & ~(1 << index);
	}

	void AddValue(bool value) {
		uint8_t length = GetTactLength();
		if (length >= MaxLength()) return;

		SetBitValue(length, value);
		SetTactLength(length++);
	}

	uint8_t MaxLength() {
		return (uint8_t)9;
	}

	uint8_t DataBytes() {
		return sizeof(data) * (uint8_t)8;
	}

	bool GetStateBit()
	{
		return GetBitValue((uint8_t)11);
	}

	void SetStateBit(bool value) {
		SetBitValue((uint8_t)11, value);
	}

	void SetFrequency(uint8_t length, uint8_t value) {
		uint16_t mask = data & 0xFF00;
		data = mask | value;
		SetTactLength(length);
	}
};


class LedController: Timer
{
private:
	unsigned long startTS = 0;
	uint16_t tactDuration = 0;
	LedBlinkFrequency blinkFreq;
	void SetLedState(uint8_t state);

public:
	LedController();
	~LedController();
	uint16_t TactDuration();
	void Loop();
	void SetFrequency(uint16_t tactDuration, uint8_t length, uint8_t frequency);
	void Stop();

	void Glow();
	void Off();
};

