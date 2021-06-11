#pragma once
#include "Stream.h"

class HardwareSerial: public Stream {
public:
    void begin(uint32_t speed, uint8_t config) {}
    void begin(uint32_t speed) {}
    bool listen() { return false; }
    void end() {}
    bool isListening() { return false; }
    bool stopListening() { return false; }
    bool overflow() { return false; }
    int peek() { return 0; }
};