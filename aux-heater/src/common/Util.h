#pragma once
#include <Arduino.h>

constexpr uint8_t AUX_RX_PIN = 3u;
constexpr uint8_t AUX_TX_PIN = 4u;

constexpr uint8_t VOLTMETER_MEASURE_PIN = A4;
constexpr uint8_t VOLTMETER_TRIGGER_PIN = 5u;

constexpr uint8_t DEBUG_RX_PIN = 6u;
constexpr uint8_t DEBUG_TX_PIN = 7u;
constexpr uint8_t DEBUG_ON_PIN = 8u;


constexpr uint32_t AUX_BAUD_RATE = 2400u;
constexpr uint32_t SERIAL_BAUD_RATE = 57600u;

constexpr uint32_t SERIAL_RESPONSE_TIMEOUT = 1000000u;

//constexpr uint32_t baudRates[] = { 1200, 2400, 4800, 9600, 14400, 19200, 28800, 57600, 115200 };

typedef bool (*StreamCallback)(Stream *);
typedef void (*StringArrayCallback)(char **, size_t);

extern uint8_t reverseByte(uint8_t x);
extern uint8_t getBitFromByte(uint8_t targetByte, uint8_t index);

extern uint8_t getBitsValue(uint8_t *target, uint8_t length, uint8_t start = 0);
extern void setBitsValue(uint8_t *target, uint8_t value, uint8_t length, uint8_t start = 0);
extern uint16_t getBitsValue(uint16_t *target, uint8_t length, uint8_t start = 0);
extern void setBitsValue(uint16_t* target, uint16_t value, uint8_t length, uint8_t start = 0);

extern bool IsBytesAreEqual(uint8_t * byteArray1, int length1, uint8_t * byteArray2, int length2);
extern void CopyByteArray(uint8_t * source, uint8_t * destination, int size);

extern size_t printBytes(char *stringBuff, size_t bufferLength, uint8_t *sendBytes, size_t byteLength);
extern size_t printLongs(char *stringBuff, size_t bufferLength, unsigned long *sendBytes, size_t byteLength);

extern size_t SplitString(char *source, char *separator, char **subStrArray, size_t arraySize, bool skipEmpty = false);
extern size_t SplitString(char *source, uint8_t separator, char **subStrArray, size_t arraySize, bool skipEmpty);

extern char *ShiftQuotations(char *quatationString);
extern void ShiftQuotations(char **subStrArray, size_t arraySize);

//[[deprecated("Replaced by outWrite(), use separate for each argument")]]
//extern void outPrintf(const char *format, ...);

extern bool isDebugListener();

extern size_t outWrite(const __FlashStringHelper *str);
extern size_t outWrite(double value, signed char width, unsigned char prec);
extern size_t outWrite(const char *str);
extern size_t outWrite(uint8_t n);
extern size_t outWrite(unsigned long n);
extern size_t outWrite(long n);
extern size_t outWrite(unsigned int n);
extern size_t outWrite(int n);
extern size_t outWrite(const char *str);
extern size_t outWrite(const uint8_t *buffer, size_t size);
extern size_t outWrite(const char *buffer, size_t size);