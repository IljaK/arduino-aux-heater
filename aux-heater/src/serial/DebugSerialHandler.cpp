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

    return writeASCII(debugStream, data, radix);
}

size_t DebugSerialHandler::outWriteASCII(long data, int radix, bool force)
{
    if (!IsSendAllowed(force)) return 0;

    return writeASCII(debugStream, data, radix);
}

void DebugSerialHandler::outWriteEnd(bool force) {
	outWrite(RESPONSE_SEPARATOR, force);
}