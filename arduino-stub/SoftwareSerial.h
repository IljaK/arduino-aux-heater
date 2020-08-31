#pragma once
#include "Stream.h"

class SoftwareSerial: public Stream
{
public:
    SoftwareSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false):Stream() {}
    ~SoftwareSerial() {}
    void begin(long speed) {}
    bool listen() { return false; }
    void end() {}
    bool isListening() { return false; }
    bool stopListening() { return false; }
    bool overflow() { return false; }
    int peek() { return 0; }
};

SoftwareSerial Serial;