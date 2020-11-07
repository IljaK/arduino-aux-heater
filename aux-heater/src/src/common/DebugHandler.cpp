#include "DebugHandler.h"

bool DebugHandler::isDebugEnabled = false;
Print *DebugHandler::debugPrint = NULL;

bool DebugHandler::IsDebugEnabled()
{ 
    return debugPrint != NULL && isDebugEnabled;
}

bool DebugHandler::IsSendAllowed(bool force)
{
    return debugPrint != NULL && (force || isDebugEnabled);
}

size_t DebugHandler::outWrite(uint8_t data, bool force)
{
	if (!IsSendAllowed(force)) return 0;
    
	return debugPrint->write(data);
}

size_t DebugHandler::outWrite(const uint8_t *buffer, size_t size, bool force)
{
	if (!IsSendAllowed(force)) return 0;
	return debugPrint->write((uint8_t *)buffer, size);
}
size_t DebugHandler::outWrite(const char *str, bool force)
{
	if (str == NULL) return 0;
	return outWrite((const uint8_t *)str, strlen(str));
}
size_t DebugHandler::outWrite(const char *buffer, size_t size, bool force)
{
	return outWrite((const uint8_t *)buffer, size);
}
size_t DebugHandler::outWrite(const __FlashStringHelper *str, bool force)
{
    if (!IsSendAllowed(force)) return 0;
    return debugPrint->print(str);
}

size_t DebugHandler::outWriteASCII(int data, int radix, bool force)
{
    if (!IsSendAllowed(force)) return 0;

    return writeASCII(debugPrint, data, radix);
}

size_t DebugHandler::outWriteASCII(long data, int radix, bool force)
{
    if (!IsSendAllowed(force)) return 0;

    return writeASCII(debugPrint, data, radix);
}

void DebugHandler::outWriteEnd(bool force) {
	outWrite(RESPONSE_SEPARATOR, force);
}