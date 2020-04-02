#pragma once
#include "common/Timer.h"
#include "common/Util.h"

struct LedBlinkFrequency
{
private:
	// Old
	// First 3 bits will store tact Length + 1 bit led state
	//uint16_t tactData = 0;

	// tactInfo uint_8
	// [x,xxx,xxxx]
	// 1 bit led state  + 3 bits current bit index + 4 bits store length
	uint8_t tactInfo = 0;
	uint16_t tactData = 0;

public:

	uint8_t GetTactLength() {
		return getBitsValue(&tactInfo, 4);

	}
	void SetTactLength(uint8_t value) {
		setBitsValue(&tactInfo, value, 4);
	}

	bool GetStateBit() {
		return getBitsValue(&tactInfo, 1, 7);
	}

	void SetStateBit(bool value) {
		setBitsValue(&tactInfo, value, 1, 7);
	}

	// TODO: uint8_t length, uint16_t value
	void SetFrequency(uint8_t length, uint16_t value) {
		tactData = value;
		setBitsValue(&tactInfo, length, 4);
	}

	uint8_t GetTactIndex()
	{
		return getBitsValue(&tactInfo, 3, 4);
	}

	void SetTactIndex(uint8_t value)
	{
		setBitsValue(&tactInfo, value, 3, 4);
	}

	bool GetBitValue(uint8_t index)
	{
		return getBitsValue(&tactData, 1, index);
	}
};


class LedController
{
private:
	uint16_t tactDuration;
	LedBlinkFrequency blinkFreq;
	void SetLedState(uint8_t state);

public:
	LedController();
	~LedController();

	uint16_t TactDuration();
	void SetFrequency(uint16_t tactDuration, uint8_t length, uint16_t frequency);
	void Stop();
	void Loop();

	void Glow();
	void Off();
};

