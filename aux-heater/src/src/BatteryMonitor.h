#pragma once
#include "Definitions.h"
#include "measurements/VoltMeter.h"
#include "measurements/AmperMeter.h"
#include "common/Util.h"
#include "common/TimeManager.h"
#include "common/Timer.h"
#include <Arduino.h>


enum class VoltageLevelState : uint8_t
{
	DEAD_LEVEL,
	CRITICAL_LEVEL,
	LOW_LEVEL,

	NORMAL_LEVEL,

	CHARGE_LEVEL,
	OVERFLOW_LEVEL
};

struct BatteryData {
	float voltage = 0;
	float pinVoltage = 0;
	float ampers = 0;
	float calcVoltage = 0;
};

//#define R1 20000.0
//#define R2 10000.0
//#define R3 10000.0
//#define R4 10000.0

//constexpr double VOLTAGE_DIVIDER = (R1 / (R1 + R2)) * (R3 / (R3 + R4));
constexpr double VOLTAGE_DIVIDER = 0.12518;

constexpr uint32_t DELAY_BEFORE_MEASURE = 2000u;
constexpr uint32_t DELAY_BETWEEN_MEASURE = 1000000u; //60000000u; // 60 sec

typedef void (*VoltageStateCallback)(VoltageLevelState);

class BatteryMonitor: public ITimerCallback
{
private:
    VoltMeter voltMeter;
    AmperMeter amperMeter;

    TimerID measureTimer = 0;
	VoltageStateCallback stateCallback = NULL;
	VoltageLevelState currentState = VoltageLevelState::NORMAL_LEVEL;

	uint8_t matchRequire = 3;
	uint8_t matchedAttempts = 0;
	VoltageLevelState matchState = VoltageLevelState::NORMAL_LEVEL;

	float GetStateVoltage(VoltageLevelState state);
	void HandleAttempt(VoltageLevelState state);
	VoltageLevelState GetNextState(VoltageLevelState state);
	bool IsNormalized();
    void StopTimer();
    void StartTimer();

protected:
    Print *debugPrint = NULL;
	void OnVoltageMeasured();

public:
	BatteryMonitor(VoltageStateCallback actionCallback = NULL);
	~BatteryMonitor();

    void SetDebugOutput(Print *debugPrint);
    void OnTimerComplete(TimerID timerId, uint8_t data) override;

	VoltageLevelState CurrentState();
    void GetBatteryData(BatteryData* data);

    double CalcVoltage();
    void Start();
};

