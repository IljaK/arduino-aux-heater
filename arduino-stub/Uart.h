#pragma once
#include "Stream.h"
#include "Sercom.h"

class Uart: public Stream {
public:
    Uart (SERCOM*, uint8_t, uint8_t, SercomRXPad, SercomUartTXPad) {};
    void IrqHandler() {}
};