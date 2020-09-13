#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "EEprom.h"

#define SERIAL_RX_BUFFER_SIZE 64

#if (SERIAL_TX_BUFFER_SIZE>256)
typedef uint16_t tx_buffer_index_t;
#else
typedef uint8_t tx_buffer_index_t;
#endif
#if  (SERIAL_RX_BUFFER_SIZE>256)
typedef uint16_t rx_buffer_index_t;
#else
typedef uint8_t rx_buffer_index_t;
#endif


class Stream
{
protected:
	rx_buffer_index_t length = 0;
	uint8_t buffer[SERIAL_RX_BUFFER_SIZE];
public:
	Stream();

	// From Aduino HardwareSerial.h
	inline size_t write(unsigned long n) { return write((uint8_t)n); }
	inline size_t write(long n) { return write((uint8_t)n); }
	inline size_t write(unsigned int n) { return write((uint8_t)n); }
	inline size_t write(int n) { return write((uint8_t)n); }
	size_t write(const __FlashStringHelper* str) { return 0; }

	size_t write(uint8_t byte) { return sizeof(byte); }
	size_t write(uint16_t data) { return write((uint8_t)data); }
	size_t write(char byte) { return write((uint8_t)byte); };
	size_t write(char *str) { return strlen(str); };
	size_t write(uint8_t *buffer, size_t length) { return length; };
	size_t write(char *str, size_t length) { return write((uint8_t *)str, length); };
	size_t write(const char *str, size_t length) { return write((uint8_t *)str, length); };

	int read();

	int available() { return length; }
	void flush() { length = 0; }

	rx_buffer_index_t readBytes(char *buffer, rx_buffer_index_t length); // read chars from stream into buffer
	rx_buffer_index_t readBytes(const char *buffer, rx_buffer_index_t length) { return readBytes((char *)buffer, length); }
	rx_buffer_index_t readBytes(uint8_t *buffer, rx_buffer_index_t length) { return readBytes((char *)buffer, length); }
};