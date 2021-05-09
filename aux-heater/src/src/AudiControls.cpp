#include "AudiControls.h"

volatile uint8_t accPinState;
volatile uint8_t btPinState;

void handleEmergencyClick(uint8_t multiClicks);
Button emergencyButton(handleEmergencyClick);
void emergencyBtnPress() {
    emergencyButton.ChangeState();
}
void handleServiceClick(uint8_t multiClicks);
Button serviceButton(handleServiceClick);
void serviceBtnPress() {
    serviceButton.ChangeState();
}

void accState() {
    accPinState = !accPinState;
}
#if defined(BT_STATE_PIN)
void btState() {
    btPinState = !btPinState;
}
#endif

void AudiControls::Start()
{
#if defined(BT_PIN_ENABLE)
    pinMode(BT_PIN_ENABLE, OUTPUT);
    digitalWrite(BT_PIN_ENABLE, LOW);
#endif

#if defined(BT_STATE_PIN)
    btPinState = initPullupPin(BT_STATE_PIN, INPUT_PULLDOWN, btState);
#endif

    emergencyButton.Start(EMERGENCY_STATE_PIN, emergencyBtnPress);
    serviceButton.Start(SERVICE_STATE_PIN, serviceBtnPress);
    accPinState = initPullupPin(ACC_STATE_PIN, INPUT_PULLDOWN, accState);

    Serial.print("BT STATE: ");
    Serial.print(btPinState);
    Serial.print("\r\n");
}
void AudiControls::Loop()
{
    emergencyButton.Loop();
    serviceButton.Loop();
}
