#pragma once
#include <Arduino.h>

constexpr uint8_t CR_ASCII_SYMBOL = 13u; // CR
constexpr uint8_t LF_ASCII_SYMBOL = 10u; // LF
constexpr uint8_t CRTLZ_ASCII_SYMBOL = 26u; // ctrl+z
constexpr uint8_t ESC_ASCII_SYMBOL = 27u; // ESC

#if ESP32
	#define AUX_RX_PIN 9
	#define AUX_TX_PIN 10

	#define GSM_RX_PIN 16
	#define GSM_TX_PIN 17

	#define VOLTMETER_MEASURE_PIN A4
	#define AMPERMETER_MEASURE_PIN A5
	#define VOLTMETER_TRIGGER_PIN 5
#else
	#define BT_RX_PIN 1
	#define BT_TX_PIN 0

	#define AUX_RX_PIN 13
	#define AUX_TX_PIN 14

	#define VOLTMETER_MEASURE_PIN A4
	#define AMPERMETER_MEASURE_PIN A5
	#define VOLTMETER_TRIGGER_PIN 5

    #define DS18S20_PIN 2
    #define BT_BAUD_RATE 115200

    #define BT_PIN_ENABLE 3
    #define BT_STATE_PIN 4
#endif

#define ACC_STATE_PIN 5
#define EMERGENCY_STATE_PIN 6
#define SERVICE_STATE_PIN 7

#if ESP32 || ARDUINO_ARCH_SAMD
#define PIN_RESOLUTION_UNITS 4096 // 12 bit size
#else
#define PIN_RESOLUTION_UNITS 1024 // 10 bit size
#endif

constexpr uint16_t PIN_RESOLUTION = PIN_RESOLUTION_UNITS - 1;

#ifndef SERIAL_CHAR_BUFFER_SIZE
#define SERIAL_CHAR_BUFFER_SIZE 64
#endif


constexpr char BT_STATS_CMD[] = "+STATS";
constexpr char RESPONSE_SEPARATOR[] = "\r\n";

#define AUX_BAUD_RATE 2400
#define SERIAL_BAUD_RATE 115200

constexpr uint32_t SERIAL_RESPONSE_TIMEOUT = 1000000u;
constexpr uint32_t GSM_CMD_DELAY = 500000u;

//constexpr uint32_t baudRates[] = { 1200, 2400, 4800, 9600, 14400, 19200, 28800, 57600, 115200 };

//temp|out temp|humidity|pressure|voltage|ampers|calculated voltage

struct BatteryData {
	float voltage = 0;
	float pinVoltage = 0;
	float ampers = 0;
	float calcVoltage = 0;
};

struct DeviceSpecData {
    uint32_t remainRam = 0;
    uint32_t activeTime = 0;
};

struct ByteArray {
    uint8_t length;
    uint8_t * array;
};

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

extern size_t writeASCII(Print *stream, int data, int radix = 10);
extern size_t writeASCII(Print *stream, unsigned int data, int radix = 10);
extern size_t writeASCII(Print *stream, long data, int radix = 10);
extern size_t writeASCII(Print *stream, unsigned long data, int radix = 10);

//[[deprecated("Replaced by outWrite(), use separate for each argument")]]
//extern void outPrintf(const char *format, ...);

extern uint32_t remainRam();
extern double readAnalogVoltage(uint8_t pin);