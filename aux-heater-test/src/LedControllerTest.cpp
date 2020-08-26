#include <gtest/gtest.h>
#include <Arduino.h>
#include <LedController.h>


TEST(LedControllerTest, LedBlinkTest)
{
    timeOffset = 0;
    
    LedController ledController;

	ledController.Glow();

    uint8_t length = 2;
    uint16_t tactDuration = 500u;

	ledController.SetFrequency(tactDuration, length, 0b00000010);

    ledController.Loop();

    for (int i = 0; i < length * 2 + 1; i++) {
        timeOffset += tactDuration / 2 * 1000ul;
        ledController.Loop();
    }
}