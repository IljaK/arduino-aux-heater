#pragma once
#include <Arduino.h>

constexpr uint8_t CR_ASCII_SYMBOL = 13u; // CR
constexpr uint8_t LF_ASCII_SYMBOL = 10u; // LF
constexpr uint8_t CRTLZ_ASCII_SYMBOL = 26u; // ctrl+z
constexpr uint8_t ESC_ASCII_SYMBOL = 27u; // ESC

constexpr uint8_t AUX_RX_PIN = 3u;
constexpr uint8_t AUX_TX_PIN = 4u;

#ifndef SERIAL_CHAR_BUFFER_SIZE
#define SERIAL_CHAR_BUFFER_SIZE 128
#endif

constexpr uint8_t VOLTMETER_MEASURE_PIN = A4;
constexpr uint8_t VOLTMETER_TRIGGER_PIN = 5u;

constexpr uint8_t DEBUG_RX_PIN = 8u;
constexpr uint8_t DEBUG_TX_PIN = 9u;

constexpr char RESPONSE_SEPARATOR[] = "\r\n";

constexpr uint32_t AUX_BAUD_RATE = 2400u;
constexpr uint32_t SERIAL_BAUD_RATE = 115200u;

constexpr uint32_t SERIAL_RESPONSE_TIMEOUT = 1000000u;

//constexpr uint32_t baudRates[] = { 1200, 2400, 4800, 9600, 14400, 19200, 28800, 57600, 115200 };

//temp|out temp|humidity|pressure|voltage|ampers|calculated voltage

struct BME280Data {
	float temperature = 0;
	float humidity = 0;
	float pressure = 0;
};

struct BatteryData {
	float voltage = 0;
	float ampers = 0;
	float calcVoltage = 0;
};

struct DeviceSpecData {
    uint32_t remainRam = 0;
	// TODO: Active time
};

struct ByteArray {
    uint8_t length;
    uint8_t * array;
};

typedef void (*BME1280DataCallback)(BME280Data *);
typedef void (*BatteryDataCallback)(BatteryData *);
typedef bool (*StreamCallback)(Stream *);
typedef void (*StringArrayCallback)(char **, size_t);
typedef void (*StringCallback)(char *, size_t);

extern uint8_t reverseByte(uint8_t x);
extern uint8_t getBitFromByte(uint8_t targetByte, uint8_t index);

extern uint8_t getBitsValue(uint8_t *target, uint8_t length, uint8_t start = 0);
extern void setBitsValue(uint8_t *target, uint8_t value, uint8_t length, uint8_t start = 0);
extern uint16_t getBitsValue(uint16_t *target, uint8_t length, uint8_t start = 0);
extern void setBitsValue(uint16_t* target, uint16_t value, uint8_t length, uint8_t start = 0);

extern bool IsByteArraysEqual(uint8_t * byteArray1, int length1, uint8_t * byteArray2, int length2);
extern void CopyByteArray(uint8_t * source, uint8_t * destination, int size);

extern size_t SplitString(char *source, char *separator, char **subStrArray, size_t arraySize, bool skipEmpty = false);
extern size_t SplitString(char *source, uint8_t separator, char **subStrArray, size_t arraySize, bool skipEmpty);

extern char *ShiftQuotations(char *quatationString);
extern void ShiftQuotations(char **subStrArray, size_t arraySize);

extern size_t writeDouble(Print *stream, double value, signed char width, unsigned char prec);
extern size_t writeASCII(Print *stream, int data, int radix = 10);
extern size_t writeASCII(Print *stream, unsigned int data, int radix = 10);
extern size_t writeASCII(Print *stream, long data, int radix = 10);
extern size_t writeASCII(Print *stream, unsigned long data, int radix = 10);

//[[deprecated("Replaced by outWrite(), use separate for each argument")]]
//extern void outPrintf(const char *format, ...);

extern uint32_t remainRam();