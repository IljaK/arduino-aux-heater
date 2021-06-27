#pragma once
#include "Timer.h"
#include "Util.h"

typedef void (*ClickFuncPtr)(uint8_t);
constexpr uint32_t MULTI_CLICK_WAIT = 300UL;
constexpr uint8_t MAX_MULTI_CLICK = 3;

class Button: public ITimerCallback
{
private:
    volatile uint8_t pinState = LOW;
    uint8_t btnState = LOW;
    ClickFuncPtr clickCallBack = NULL;
    TimerID multiClickTimer = 0;
    uint8_t multiClicks = 0;

    inline void OnBtnReleased();
    inline void OnBtnPressed();
    inline void ProcessClick();
public:
    Button(ClickFuncPtr clickCallBack);
    void Start(uint8_t pin, voidFuncPtr func);
    void Loop();
    inline void ChangeState() { pinState = !pinState; }
    bool IsPressed();

    void OnTimerComplete(TimerID timerId, uint8_t data) override;
    void OnTimerStop(TimerID timerId, uint8_t data) override;
};