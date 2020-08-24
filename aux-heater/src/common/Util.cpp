#include "Util.h"
#include <stdarg.h>
#pragma warning(disable : 4996)

Stream *outStream = NULL;

inline void tunedDelay(uint16_t duration)
{
#ifdef ARDUINO
	uint8_t tmp = 0;

	asm volatile("sbiw    %0, 0x01 \n\t"
	"ldi %1, 0xFF \n\t"
	"cpi %A0, 0xFF \n\t"
	"cpc %B0, %1 \n\t"
	"brne .-10 \n\t"
	: "+w" (duration), "+a" (tmp)
	: "0" (duration)
	);
#else 
	delay(duration);
#endif
}

uint8_t reverseByte(uint8_t x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	return((x >> 8) | (x << 8));
}

uint8_t getBitFromByte(uint8_t targetByte, uint8_t index)
{
	return (targetByte >> index) & 1;
}

uint8_t getBitsValue(uint8_t * target, uint8_t length, uint8_t start)
{
	uint8_t headerShift = (BITS_PER_BYTE - start - length);
	uint8_t result = *target;
	result = result << headerShift;
	result = result >> (headerShift + start);

	return result;
}

uint16_t getBitsValue(uint16_t * target, uint8_t length, uint8_t start)
{
	uint16_t headerShift = ((BITS_PER_BYTE * 2) - start - length);
	uint16_t result = *target;
	result = result << headerShift;
	result = result >> (headerShift + start);

	return result;
}

void setBitsValue(uint8_t * target, uint8_t value, uint8_t length, uint8_t start)
{
	uint8_t headerShift = (BITS_PER_BYTE - start - length);

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
	uint16_t headerShift = ((BITS_PER_BYTE * 2) - start - length);

	uint16_t mask = 0xFFFF << (start + headerShift);
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


bool IsBytesAreEqual(uint8_t * byteArray1, int length1, uint8_t * byteArray2, int length2)
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

size_t printBytes(char *stringBuff, size_t bufferLength, uint8_t *sendBytes, size_t byteLength)
{
	if (byteLength > 0 && bufferLength > 0)
	{
		stringBuff[0] = '\0';
		char buffer[8];
		buffer[0] = '\0';

		int outPrinted = 1;

		for (size_t i = 0; i < byteLength; i++) {
			snprintf(buffer, 8, "%d", (int)sendBytes[i]);

			size_t toPrint = strlen(buffer);
			if (i > 0) toPrint++;

			if (outPrinted + toPrint > bufferLength) return i;

			outPrinted += toPrint;

			if (i > 0) {
				strncat(stringBuff, "-", bufferLength);
			}
			strncat(stringBuff, buffer, bufferLength);
		}
		return byteLength;
	}
	return 0;
}

size_t printLongs(char *stringBuff, size_t bufferLength, unsigned long *sendBytes, size_t byteLength)
{
	if (byteLength > 0 && bufferLength > 0)
	{
		stringBuff[0] = '\0';
		char buffer[32];
		buffer[0] = '\0';

		int outPrinted = 1;

		for (size_t i = 0; i < byteLength; i++) {
			snprintf(buffer, 32, "%lu", sendBytes[i]);

			size_t toPrint = strlen(buffer);
			if (i > 0) toPrint++;

			if (outPrinted + toPrint > bufferLength) return i;
			outPrinted += toPrint;

			if (i > 0) strncat(stringBuff, "-", bufferLength);
			strncat(stringBuff, buffer, bufferLength);
		}
		return byteLength;
	}
	return 0;
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

		if (!(separator[0] != QUOTATION && separator[1] != 0)) {
			if (inQuotation) {
				if (pChr[0] == QUOTATION) {
					inQuotation = false;
				}
				pChr++;
				continue;
			}
			else {
				if (pChr[0] == QUOTATION) {
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
			size_t argLen = strlen(subStrArray[i]);
			if (subStrArray[i][0] == QUOTATION && subStrArray[i][argLen - 1] == QUOTATION)
			{
				subStrArray[i] += 1;
				subStrArray[i][argLen - 2] = 0;
			}
		}
	}
}

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

	outWrite("\r\n", 2);
}

size_t outWrite(uint8_t data) {
	if (!outStream) return 0;
	return outStream->write(data);
}
size_t outWrite(unsigned long n) { return outWrite((uint8_t)n); }
size_t outWrite(long n) { return outWrite((uint8_t)n); }
size_t outWrite(unsigned int n) { return outWrite((uint8_t)n); }
size_t outWrite(int n) { return outWrite((uint8_t)n); }

size_t outWrite(const uint8_t *buffer, size_t size) {
	if (!outStream) return 0;
	return outStream->write((uint8_t *)buffer, size);
}
size_t outWrite(const char *str) {
	if (str == NULL) return 0;
	return outWrite((const uint8_t *)str, strlen(str));
}
size_t outWrite(const char *buffer, size_t size) {
	return outWrite((const uint8_t *)buffer, size);
}

