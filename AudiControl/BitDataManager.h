#pragma once
#include <Arduino.h>

constexpr uint8_t MAX_BITS_AMOUNT = 8;
constexpr uint8_t BIT_TABLE_LENGTH = 48;

struct BitData
{
private:
	// First bit will store true/false, other unsigned short bit values
	uint16_t duration = 0;
public:

	uint16_t Duration() {
		// Get rid of first bit
		return ((uint16_t)(duration << 1) >> 1);
	}
	void Duration(uint16_t value) {
		uint16_t mask = duration & 0x8000;
		duration = (((uint16_t)(value << 1) >> 1) | mask);
	}

	bool IsPositive() {
		return (duration >> 15) == 1;
	}

	void IsPositive(bool value) {
		uint16_t mask = 0x0000;
		if (value) mask = 0x8000;
		duration = Duration() | mask;
	}
	uint8_t Value() {
		return IsPositive() ? HIGH : LOW;
	}
};

struct BitRow
{
	BitData bitData[MAX_BITS_AMOUNT];
	unsigned long bitRowBeginTS = 0;

private:
	// First bit will store true/false, other unsigned short bit values
	uint8_t length = LOW;
public:

	uint8_t Length() {
		// Get rid of first bit
		return ((uint8_t)(length << 1) >> 1);
	}
	void Length(uint8_t value) {
		uint8_t mask = length & 0x80;
		length = (((uint8_t)(value << 1) >> 1) | mask);
	}

	bool IsOwerFlow() {
		return (length >> 7) == 1;
	}

	void IsOwerFlow(bool value) {
		uint8_t mask = LOW;
		if (value) mask = 0x80;
		length = Length() | mask;
	}
};

class BitDataManager
{
private:
	BitRow bitTable[BIT_TABLE_LENGTH];
public:
	BitDataManager();
	~BitDataManager();

	void FlushBitTable();
	uint8_t ActualTableIndex();
	uint8_t TableSize();
	bool AppendBitDta(uint8_t pinValue, bool newBitRow, unsigned long timeStamp);
	unsigned long BitArrayDuration(BitRow *bitRow);

	BitRow *BitTableRow(uint8_t index);
	unsigned long MaxBitDuration();
};

