#include "Button.h"

Button::Button(ClickFuncPtr clickCallBack):ITimerCallback()
{
    this->clickCallBack = clickCallBack;
}
void Button::Start(uint8_t pin, voidFuncPtr func)
{
    pinState = initPullupPin(pin, INPUT_PULLUP, func);
    btnState = pinState;
}

void Button::Loop()
{
    if (btnState != pinState) {
        if (pinState == LOW) {
            OnBtnPressed();
        } else {
            OnBtnReleased();
        }
    }
}
void Button::OnBtnReleased()
{
    if (multiClickTimer != 0) {
        Timer::Stop(multiClickTimer);
        multiClickTimer = 0;
    }
    multiClicks++;
    multiClickTimer = Timer::Start(this, MULTI_CLICK_WAIT);
}
void Button::ProcessClick()
{
    uint8_t clicks = multiClicks;
    if (clicks >= MAX_MULTI_CLICK) {
        clicks = MAX_MULTI_CLICK;
    }
    multiClicks = 0;
    if (clickCallBack != NULL) {
        clickCallBack(clicks);
    }
}
void Button::OnBtnPressed()
{
    if (multiClickTimer != 0) {
        Timer::Stop(multiClickTimer);
        multiClickTimer = 0;
    }
}

bool Button::IsPressed()
{
    return btnState == LOW;
}

void Button::OnTimerComplete(TimerID timerId, uint8_t data)
{
    if (timerId == multiClickTimer) {
        multiClickTimer = 0;
        ProcessClick();
    }
}