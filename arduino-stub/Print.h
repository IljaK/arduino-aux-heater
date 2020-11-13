#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "EEprom.h"

class Print
{
public:
	Print() {};

	// From Aduino HardwareSerial.h
	inline size_t write(unsigned long n) { return write((uint8_t)n); }
	inline size_t write(long n) { return write((uint8_t)n); }
	inline size_t write(unsigned int n) { return write((uint8_t)n); }
	inline size_t write(int n) { return write((uint8_t)n); }
	size_t print(const __FlashStringHelper* str) { return 0; }

	size_t write(uint8_t byte) { return sizeof(byte); }
	size_t write(uint16_t data) { return write((uint8_t)data); }
	size_t write(char byte) { return write((uint8_t)byte); };
	size_t write(char *str) { return strlen(str); };
	size_t write(uint8_t *buffer, size_t length) { return length; };
	size_t write(char *str, size_t length) { return write((uint8_t *)str, length); };
	size_t write(const char *str, size_t length) { return write((uint8_t *)str, length); };
	size_t write(const char *str) { return write((uint8_t *)str, strlen(str)); };

	virtual int read() = 0;
	virtual int available() = 0;
};