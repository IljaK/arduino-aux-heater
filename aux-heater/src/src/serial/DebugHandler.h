#pragma once
#include <Arduino.h>
#include "../common/Util.h"
#include "SerialCharResponseHandler.h"

class DebugHandler
{
protected:
    static bool isDebugEnabled;
    static Print *debugPrint;

    static bool IsSendAllowed(bool force);
public:
    static void SetPrint(Print *debugPrint);    
    static bool IsDebugEnabled();

    static size_t outWrite(const __FlashStringHelper *str, bool force = false);
    static size_t outWrite(const char *str, bool force = false);
    static size_t outWrite(uint32_t n, bool force = false);
    static size_t outWrite(const uint8_t *buffer, size_t size, bool force = false);
    static size_t outWrite(const char *buffer, size_t size, bool force = false);

    static size_t outWriteASCII(int data, int radix = 10, bool force = false);
    static size_t outWriteASCII(long data, int radix = 10, bool force = false);
    static void outWriteEnd(bool force = false);
};