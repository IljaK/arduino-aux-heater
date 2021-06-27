#pragma once
#include <Arduino.h>

#if defined(ESP32) || defined(ARDUINO_ARCH_SAMD)
#define ANALOG_BITS_RESOLUTION 12 // 12 bits = 4096
#define ANALOG_UNITS_RESOLUTION 4096
#else
#define ANALOG_BITS_RESOLUTION 10 // 10 bits = 1024
#define ANALOG_UNITS_RESOLUTION 1024 // 10 bit size
#endif

constexpr uint16_t ANALOG_UNITS_RESOLUTION_RANGE = ANALOG_UNITS_RESOLUTION - 1;

constexpr uint8_t CRTLZ_ASCII_SYMBOL = 26u; // ctrl+z
constexpr uint8_t ESC_ASCII_SYMBOL = 27u; // ESC

//constexpr uint32_t baudRates[] = { 1200, 2400, 4800, 9600, 14400, 19200, 28800, 57600, 115200 };
//temp|out temp|humidity|pressure|voltage|ampers|calculated voltage

extern void(* rebootDeviceFunc) (void);

struct DeviceSpecData {
    uint32_t remainRam = 0;
    uint32_t activeTime = 0;
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

//[[deprecated("Replaced by outWrite(), use separate for each argument")]]
//extern void outPrintf(const char *format, ...);

extern uint32_t remainRam();
extern double readAnalogVoltage(uint8_t pin);
extern uint8_t initPullupPin(uint8_t pin, PinMode mode, voidFuncPtr isrFunc);

// outBuffer must be min size of: 2 * length + 1
extern size_t encodeToHex(uint8_t *inArray, size_t length, char *outBuffer);
extern size_t decodeFromHex(char *inBuffer, uint8_t *outArray, size_t length);