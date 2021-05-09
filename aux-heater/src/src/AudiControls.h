#pragma once
#include "Definitions.h"
#include "../src/common/Button.h"

//extern Button emergencyButton;
//extern Button serviceButton;
//extern volatile uint8_t accPinState;
//extern volatile uint8_t btPinState;

class AudiControls
{
public:
    void Start();
    void Loop();
};