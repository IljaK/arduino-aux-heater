#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "EEprom.h"


#define DEC 10
#define HEX 16
#define OCT 8
#ifdef BIN // Prevent warnings if BIN is previously defined in "iotnx4.h" or similar
#undef BIN
#endif
#define BIN 2

class Print
{
public:
	Print() {};

	// From Aduino HardwareSerial.h
	size_t write(uint8_t byte) { return sizeof(byte); }
	size_t write(const uint8_t *buffer, size_t length) { return length; };
    size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }

	inline size_t print(unsigned long n, int = DEC) { return write((uint8_t)n); }
	inline size_t print(long n, int = DEC) { return write((uint8_t)n); }
	inline size_t print(unsigned int n, int = DEC) { return write((uint8_t)n); }
	inline size_t print(int n, int = DEC) { return write((uint8_t)n); }

	size_t print(const __FlashStringHelper* str) { return 0; }
	size_t print(char byte) { return write((uint8_t)byte); };
	size_t print(char *str, size_t length) { return write((uint8_t *)str, length); };
	size_t print(const char *str, size_t length) { return write((uint8_t *)str, length); };
	size_t print(const char *str) { return write((uint8_t *)str, strlen(str)); };

    inline size_t println(unsigned long n, int = DEC) { return write((uint8_t)n); }
	inline size_t println(long n, int = DEC) { return write((uint8_t)n); }
	inline size_t println(unsigned int n, int = DEC) { return write((uint8_t)n); }
	inline size_t println(int n, int = DEC) { return write((uint8_t)n); }

	size_t println(const __FlashStringHelper* str) { return 0; }
	size_t println(char byte) { return write((uint8_t)byte); };
	size_t println(char *str, size_t length) { return write((uint8_t *)str, length); };
	size_t println(const char *str, size_t length) { return write((uint8_t *)str, length); };
	size_t println(const char *str) { return write((uint8_t *)str, strlen(str)); };

	virtual int read() = 0;
	virtual int available() = 0;
};