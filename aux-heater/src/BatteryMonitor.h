#pragma once
#include "common/VoltMeter.h"
#include "common/Util.h"
#include "common/TimeUtil.h"
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

class BatteryMonitor : public VoltMeter
{
private:
	void(*stateCallback)(VoltageLevelState) = NULL;
	VoltageLevelState currentState = VoltageLevelState::NORMAL_LEVEL;

	uint8_t matchRequire = 3;
	uint8_t matchedAttempts = 0;
	VoltageLevelState matchState = VoltageLevelState::NORMAL_LEVEL;

	uint8_t valueLevel[6] = { 105, 118, 120, 125, 145, 155 };

	float GetStateVoltage(VoltageLevelState state);
	void HandleAttempt(VoltageLevelState state);
	VoltageLevelState GetNextState(VoltageLevelState state);
	bool IsNormalized();

protected:
	void OnVoltageMeasured() override;

public:

	VoltageLevelState CurrentState();

	BatteryMonitor(float r1, float r2, void(*actionCallback)(VoltageLevelState) = NULL);
	~BatteryMonitor();
};

