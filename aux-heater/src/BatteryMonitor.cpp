#include "BatteryMonitor.h"


BatteryMonitor::BatteryMonitor(float r1, float r2, void(*stateCallback)(VoltageLevelState)):VoltMeter(r1, r2)
{
	this->stateCallback = stateCallback;
	StartMeasureTimer(VoltMeterState::WAIT_NEXT, DELAY_BETWEEN_MEASURE);
}

BatteryMonitor::~BatteryMonitor()
{
}

VoltageLevelState BatteryMonitor::CurrentState()
{
	return currentState;
}

void BatteryMonitor::OnVoltageMeasured()
{
	char resultVoltage[16];
	dtostrf(PinVoltage(), 5, 3, resultVoltage);
	outPrintf("PIN Voltage: %s", resultVoltage);
	
	dtostrf(Voltage(), 5, 3, resultVoltage);
	outPrintf("Voltage: %s", resultVoltage);

	VoltageLevelState nextState = GetNextState(currentState);
	if (nextState == currentState) {
		// Check if normalized back
		if (IsNormalized()) {
			HandleAttempt(VoltageLevelState::NORMAL_LEVEL);
		}
	}
	else {
		if (currentState == VoltageLevelState::NORMAL_LEVEL) {
			if (Voltage() >= GetStateVoltage(VoltageLevelState::OVERFLOW_LEVEL)) {
				HandleAttempt(VoltageLevelState::OVERFLOW_LEVEL);
			}
			else if (Voltage() <= GetStateVoltage(VoltageLevelState::LOW_LEVEL)) {
				HandleAttempt(VoltageLevelState::LOW_LEVEL);
			}

		}
		else if (Voltage() <= GetStateVoltage(nextState)) {
			HandleAttempt(nextState);
		}
		else if (IsNormalized()) {
			HandleAttempt(VoltageLevelState::NORMAL_LEVEL);
		}
		else {
			matchedAttempts = 0;
			matchState = currentState;
		}
	}
}

bool BatteryMonitor::IsNormalized()
{
	return Voltage() >= GetStateVoltage(VoltageLevelState::NORMAL_LEVEL) && Voltage() < GetStateVoltage(VoltageLevelState::CHARGE_LEVEL);
}

VoltageLevelState BatteryMonitor::GetNextState(VoltageLevelState state)
{
	if (state == VoltageLevelState::OVERFLOW_LEVEL) return VoltageLevelState::OVERFLOW_LEVEL;
	if (state == VoltageLevelState::DEAD_LEVEL) return VoltageLevelState::DEAD_LEVEL;
	uint8_t valueLevel = (uint8_t)state;
	valueLevel--;
	return (VoltageLevelState)valueLevel;
}

void BatteryMonitor::HandleAttempt(VoltageLevelState state)
{
	if (matchState != state) {
		matchedAttempts = 0;
		matchState = state;
	}
	matchedAttempts++;
	if (matchedAttempts >= matchRequire) {
		outPrintf("Handle Battery LEVEL: %d", (int)state);
		currentState = state;
		matchedAttempts = 0;
		// TODO: Trigger message send && reset timer
		StartMeasureTimer(VoltMeterState::WAIT_NEXT, DELAY_ATTEMPT_MEASURE);
		if (stateCallback != NULL) {
			stateCallback(currentState);
		}
	}
}

float BatteryMonitor::GetStateVoltage(VoltageLevelState state)
{
	return ((float)valueLevel[(uint8_t)state]) / 10.0f;
}
