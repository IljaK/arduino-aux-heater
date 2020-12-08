#include "Util.h"
#include <stdarg.h>
#include <string.h>
//#pragma warning(disable : 4996)

Stream *outStream = NULL;

uint8_t reverseByte(uint8_t x)
{
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	return x;
}

uint8_t getBitFromByte(uint8_t targetByte, uint8_t index)
{
	return (targetByte >> index) & 1;
}

uint8_t getBitsValue(uint8_t * target, uint8_t length, uint8_t start)
{
	uint8_t headerShift = (8u - start - length);
	uint8_t result = *target;
	result = result << headerShift;
	result = result >> (headerShift + start);

	return result;
}

uint16_t getBitsValue(uint16_t * target, uint8_t length, uint8_t start)
{
	uint16_t headerShift = ((16u) - start - length);
	uint16_t result = *target;
	result = result << headerShift;
	result = result >> (headerShift + start);

	return result;
}

void setBitsValue(uint8_t * target, uint8_t value, uint8_t length, uint8_t start)
{
	uint8_t headerShift = (8u - start - length);

	uint8_t mask = 0xFF << (start + headerShift);
	mask = mask >> headerShift;
	mask = ~mask;

	uint8_t result = *target;

	// Flush require bits
	result = result & mask;

	// Flush value header + shift to position
	value = (value << (headerShift + start)) >> headerShift;

	// Store bits
	result = result | value;

	*target = result;
}

void setBitsValue(uint16_t* target, uint16_t value, uint8_t length, uint8_t start)
{
	uint16_t headerShift = ((16u) - start - length);

	uint16_t mask = 0xFFFF << (start + headerShift);
	mask = mask >> headerShift;
	mask = ~mask;

	uint16_t result = *target;

	// Flush require bits
	result = result & mask;

	// Flush value header + shift to position
	value = (value << (headerShift + start)) >> headerShift;

	// Store bits
	result = result | value;

	*target = result;
}


bool IsByteArraysEqual(uint8_t * byteArray1, int length1, uint8_t * byteArray2, int length2)
{
	if (length1 != length2 || length1 == 0) return false;
	for (int i = 0; i < length1; i++) {
		if (byteArray1[i] != byteArray2[i]) return false;
	}
	return true;
}

void CopyByteArray(uint8_t * source, uint8_t * destination, int size)
{
	if (size == 0) return;
	for (int i = 0; i < size; i++) {
		destination[i] = source[i];
	}
}

size_t SplitString(char *source, uint8_t separator, char **subStrArray, size_t arraySize, bool skipEmpty)
{
	char sep[2] = { (char)separator , 0 };
	return SplitString(source, sep, subStrArray, arraySize, skipEmpty);
}

size_t SplitString(char *source, char *separator, char **subStrArray, size_t arraySize, bool skipEmpty)
{
	if (arraySize == 0) return 0;

	bool inQuotation = false;
	size_t separatorLength = strlen(separator);
	char *pChr = source;

	subStrArray[0] = pChr;
	size_t argumentsAmount = 1;

	if (argumentsAmount >= arraySize) {
		return argumentsAmount;
	}

	while (pChr[0] != 0) {

		if (!(separator[0] != '\"' && separator[1] != 0)) {
			if (inQuotation) {
				if (pChr[0] == '\"') {
					inQuotation = false;
				}
				pChr++;
				continue;
			}
			else {
				if (pChr[0] == '\"') {
					inQuotation = true;
					pChr++;
					continue;
				}
			}
		}

		if (strncmp(pChr, separator, separatorLength) == 0) {
			char *nextArg = pChr + separatorLength;
			pChr[0] = 0;
			pChr = nextArg;

			if (skipEmpty) {
				if (subStrArray[argumentsAmount - 1][0] == 0) {
					argumentsAmount--;
				}
			}

			if (nextArg[0] == 0) {
				break;
			}
			else {
				subStrArray[argumentsAmount] = nextArg;
				argumentsAmount++;
			}

			if (argumentsAmount >= arraySize) {
				break;
			}
			continue;
		}

		pChr++;
	}
	return argumentsAmount;
}

void ShiftQuotations(char **subStrArray, size_t arraySize)
{
	for (size_t i = 0; i < arraySize; i++) {
		if (subStrArray[i] != 0) {
			subStrArray[i] = ShiftQuotations(subStrArray[i]);
		}
	}
}

char *ShiftQuotations(char *quatationString)
{
	size_t length = strlen(quatationString);
	if (quatationString[0] == '\"' && quatationString[length - 1] == '\"')
	{
		quatationString += 1;
		quatationString[length - 2] = 0;
	}
	return quatationString;
}

uint32_t remainRam () {
#if ARDUINO_TEST
	return 0;
#else
	#if ESP32
		return xPortGetFreeHeapSize();
	#else
		extern int __heap_start, *__brkval;
		int v;
		return (size_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
	#endif
#endif
}
/*
void outPrintf(const char *format, ...)
{
	if (digitalRead(DEBUG_ON_PIN) == LOW) return;

	char outMessage[96];

	va_list arglist;
	va_start(arglist, format);
	vsnprintf(outMessage, 96, format, arglist);
	va_end(arglist);

	char outPart[SERIAL_RX_BUFFER_SIZE];

	int printPartLength = 0;
	char *printBegin = outMessage;
	int remainLength = strlen(printBegin);

	do {
		if (remainLength >= SERIAL_RX_BUFFER_SIZE) printPartLength = SERIAL_RX_BUFFER_SIZE - 1;
		else printPartLength = remainLength;

		strncpy(outPart, printBegin, printPartLength);
		outPart[printPartLength] = 0;
		outWrite(outPart, printPartLength);

		printBegin += printPartLength;
		remainLength -= printPartLength;

	} while (remainLength > 0);

	outEnd();
}
*/

size_t writeASCII(Print *stream, int data, int radix)
{
    const size_t size = 16;
    char outString[size];
    outString[0] = 0;
    itoa(data, outString, radix);
    return stream->write((const char *)outString);
}

size_t writeASCII(Print *stream, unsigned int data, int radix)
{
    const size_t size = 8;
    char outString[size];
    outString[0] = 0;
    utoa(data, outString, radix);
    return stream->write((const char *)outString);
}

size_t writeASCII(Print *stream, long data, int radix)
{
    const size_t size = 8;
    char outString[size];
    outString[0] = 0;
    ltoa(data, outString, radix);
    return stream->write((const char *)outString);
}

size_t writeASCII(Print *stream, unsigned long data, int radix)
{
    const size_t size = 32;
    char outString[size];
    outString[0] = 0;
    ultoa(data, outString, radix);
    return stream->write((const char *)outString);
}

/*
#ifndef ESP32
char * dtostrf(double number, signed char width, unsigned char prec, char *s) {
    bool negative = false;

    if (isnan(number)) {
        strcpy(s, "nan");
        return s;
    }
    if (isinf(number)) {
        strcpy(s, "inf");
        return s;
    }

    char* out = s;

    int fillme = width; // how many cells to fill for the integer part
    if (prec > 0) {
        fillme -= (prec+1);
    }

    // Handle negative numbers
    if (number < 0.0) {
        negative = true;
        fillme--;
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    // I optimized out most of the divisions
    double rounding = 2.0;
    for (uint8_t i = 0; i < prec; ++i)
        rounding *= 10.0;
    rounding = 1.0 / rounding;

    number += rounding;

    // Figure out how big our number really is
    double tenpow = 1.0;
    int digitcount = 1;
    while (number >= 10.0 * tenpow) {
        tenpow *= 10.0;
        digitcount++;
    }

    number /= tenpow;
    fillme -= digitcount;

    // Pad unused cells with spaces
    while (fillme-- > 0) {
        *out++ = ' ';
    }

    // Handle negative sign
    if (negative) *out++ = '-';

    // Print the digits, and if necessary, the decimal point
    digitcount += prec;
    int8_t digit = 0;
    while (digitcount-- > 0) {
        digit = (int8_t)number;
        if (digit > 9) digit = 9; // insurance
        *out++ = (char)('0' | digit);
        if ((digitcount == prec) && (prec > 0)) {
            *out++ = '.';
        }
        number -= digit;
        number *= 10.0;
    }

    // make sure the string is terminated
    *out = 0;
    return s;
}
#endif
*/