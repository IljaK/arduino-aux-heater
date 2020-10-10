#include "DebugSerialHandler.h"

bool DebugSerialHandler::isDebugEnabled = false;
Stream *DebugSerialHandler::debugStream = NULL;

bool DebugSerialHandler::IsDebugEnabled()
{ 
    return debugStream != NULL && isDebugEnabled;
}

bool DebugSerialHandler::IsSendAllowed(bool force)
{
    return debugStream != NULL && (force || isDebugEnabled);
}

size_t DebugSerialHandler::outWrite(uint8_t data, bool force)
{
	if (!IsSendAllowed(force)) return 0;
    
	return debugStream->write(data);
}

size_t DebugSerialHandler::outWrite(double value, signed char width, unsigned char prec, bool force)
{
	if (!IsSendAllowed(force)) return 0;

    const size_t size = 16;
    char outString[size];
    if (width > size) {
        width = size - 1;
    }
	dtostrf(value, width, prec, outString);

    return outWrite(outString);
}
size_t DebugSerialHandler::outWrite(unsigned long n, bool force) { return outWrite((uint8_t)n, force); }
size_t DebugSerialHandler::outWrite(long n, bool force) { return outWrite((uint8_t)n, force); }
size_t DebugSerialHandler::outWrite(unsigned int n, bool force) { return outWrite((uint8_t)n, force); }
size_t DebugSerialHandler::outWrite(int n, bool force) { return outWrite((uint8_t)n, force); }

size_t DebugSerialHandler::outWrite(const uint8_t *buffer, size_t size, bool force)
{
	if (!IsSendAllowed(force)) return 0;
	return debugStream->write((uint8_t *)buffer, size);
}
size_t DebugSerialHandler::outWrite(const char *str, bool force)
{
	if (str == NULL) return 0;
	return outWrite((const uint8_t *)str, strlen(str));
}
size_t DebugSerialHandler::outWrite(const char *buffer, size_t size, bool force)
{
	return outWrite((const uint8_t *)buffer, size);
}
size_t DebugSerialHandler::outWrite(const __FlashStringHelper *str, bool force)
{
    if (!IsSendAllowed(force)) return 0;
    return debugStream->print(str);
}

size_t DebugSerialHandler::outWriteASCII(int data, int radix, bool force)
{
    if (!IsSendAllowed(force)) return 0;

    const size_t size = 16;
    char outString[size];
    outString[0] = 0;
    itoa(data, outString, radix);
    return debugStream->write((const char *)outString);
}

size_t DebugSerialHandler::outWriteASCII(long data, int radix, bool force)
{
    if (!IsSendAllowed(force)) return 0;

    const size_t size = 16;
    char outString[size];
    outString[0] = 0;
    utoa(data, outString, radix);
    return debugStream->write((const char *)outString);
}

void DebugSerialHandler::outWriteEnd(bool force) {
	outWrite(RESPONSE_SEPARATOR, force);
}

void DebugSerialHandler::PrintBytes(uint8_t *byteArray, size_t length)
{
	if (length > 0)
	{
		for (size_t i = 0; i < length; i++) {
            outWriteASCII(byteArray[i]);
		}
	}
}

void DebugSerialHandler::PrintLongs(long *longArray, size_t length)
{
	if (length > 0)
	{
		for (size_t i = 0; i < length; i++) {
            outWriteASCII(longArray[i]);
		}
	}
}