#pragma once
#include <Arduino.h>
#include "../common/Util.h"
#include "../common/TimeUtil.h"
#include "SerialCharResponseHandler.h"

class DebugSerialHandler
{
protected:
    static bool isDebugEnabled;
    static Stream *debugStream;

    static bool IsSendAllowed(bool force);
public:
    static bool IsDebugEnabled();

    static size_t outWrite(const __FlashStringHelper *str, bool force = false);
    static size_t outWrite(double value, signed char width, unsigned char prec, bool force = false);
    static size_t outWrite(const char *str, bool force = false);
    static size_t outWrite(uint8_t n, bool force = false);
    static size_t outWrite(unsigned long n, bool force = false);
    static size_t outWrite(long n, bool force = false);
    static size_t outWrite(unsigned int n, bool force = false);
    static size_t outWrite(int n, bool force = false);
    static size_t outWrite(const uint8_t *buffer, size_t size, bool force = false);
    static size_t outWrite(const char *buffer, size_t size, bool force = false);

    static void PrintBytes(uint8_t *byteArray, size_t length);
    static void PrintLongs(long *longArray, size_t length);

    static size_t outWriteASCII(int data, int radix = 10, bool force = false);
    static size_t outWriteASCII(long data, int radix = 10, bool force = false);
    static void outWriteEnd(bool force = false);
};