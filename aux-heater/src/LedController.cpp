#include "LedController.h"

LedController::LedController()
{
	SetLedState(LOW);
}

LedController::~LedController()
{
}

void LedController::Loop()
{
	if (tactDuration > 0) {

		unsigned long passed = millis() - startTS;
		unsigned long loopDuration = (unsigned long)tactDuration * (unsigned long)blinkFreq.GetTactLength();

		while (passed > loopDuration) {
			passed -= loopDuration;
			startTS += loopDuration;
		}

		unsigned long index = (passed / (unsigned long)tactDuration);

		uint8_t bit = (uint8_t)blinkFreq.GetBitValue((uint16_t)index);

		if (bit != (uint8_t)blinkFreq.GetStateBit()) {
			SetLedState(bit);
		}
	}
}

void LedController::Stop()
{
	tactDuration = 0;
	startTS = 0;
	Off();
}

void LedController::SetFrequency(uint16_t tactDuration, uint8_t length, uint8_t frequency)
{
	startTS = millis();
	this->tactDuration = tactDuration;
	blinkFreq.SetFrequency(length, frequency);
}

void LedController::SetLedState(uint8_t state)
{
	digitalWrite(LED_BUILTIN, state);
	blinkFreq.SetStateBit(state);
}

uint16_t LedController::TactDuration()
{
	return tactDuration;
}
void LedController::Glow()
{
	Stop();
	SetLedState(HIGH);
}
void LedController::Off()
{
	Stop();
	SetLedState(LOW);
}
