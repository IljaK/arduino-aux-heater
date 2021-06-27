#include "BatteryMonitor.h"

uint8_t valueLevel[6] = { 105, 118, 120, 125, 145, 155 };

BatteryMonitor::BatteryMonitor(VoltageStateCallback stateCallback):ITimerCallback(), 
    voltMeter(VOLTAGE_DIVIDER, VOLTMETER_MEASURE_PIN),
    amperMeter(AMPERMETER_MEASURE_PIN)
{
	this->stateCallback = stateCallback;
}

BatteryMonitor::~BatteryMonitor()
{
}

void BatteryMonitor::Start()
{
    StartTimer();
}

VoltageLevelState BatteryMonitor::CurrentState()
{
	return currentState;
}

void BatteryMonitor::OnVoltageMeasured()
{
	VoltageLevelState nextState = GetNextState(currentState);
	if (nextState == currentState) {
		// Check if normalized back
		if (IsNormalized()) {
			HandleAttempt(VoltageLevelState::NORMAL_LEVEL);
		}
	}
	else {
		if (currentState == VoltageLevelState::NORMAL_LEVEL) {
			if (CalcVoltage() >= GetStateVoltage(VoltageLevelState::OVERFLOW_LEVEL)) {
				HandleAttempt(VoltageLevelState::OVERFLOW_LEVEL);
			}
			else if (CalcVoltage() <= GetStateVoltage(VoltageLevelState::LOW_LEVEL)) {
				HandleAttempt(VoltageLevelState::LOW_LEVEL);
			}

		}
		else if (CalcVoltage() <= GetStateVoltage(nextState)) {
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
	return CalcVoltage() >= GetStateVoltage(VoltageLevelState::NORMAL_LEVEL) && CalcVoltage() < GetStateVoltage(VoltageLevelState::CHARGE_LEVEL);
}

VoltageLevelState BatteryMonitor::GetNextState(VoltageLevelState state)
{
	if (state == VoltageLevelState::OVERFLOW_LEVEL) return VoltageLevelState::OVERFLOW_LEVEL;
	if (state == VoltageLevelState::DEAD_LEVEL) return VoltageLevelState::DEAD_LEVEL;
	uint8_t stateLevel = (uint8_t)state;
	stateLevel--;
	return (VoltageLevelState)stateLevel;
}

void BatteryMonitor::HandleAttempt(VoltageLevelState state)
{
	if (matchState != state) {
		matchedAttempts = 0;
		matchState = state;
	}
	matchedAttempts++;
	if (matchedAttempts >= matchRequire) {
        if (debugPrint != NULL) {
            debugPrint->print("Handle Battery LEVEL: ");
            debugPrint->println((uint8_t)state);
        }
		currentState = state;
		matchedAttempts = 0;
		// TODO: Trigger message send && reset timer
		//StartMeasureTimer(VoltMeterState::WAIT_NEXT, DELAY_ATTEMPT_MEASURE);
		if (stateCallback != NULL) {
			stateCallback(currentState);
		}
	}
}

void BatteryMonitor::OnTimerComplete(TimerID timerId, uint8_t data)
{
    if (timerId == measureTimer) {
        measureTimer = 0;
        voltMeter.Measure();
        amperMeter.Measure();
        OnVoltageMeasured();
        StartTimer();
    }
}
void BatteryMonitor::OnTimerStop(TimerID timerId, uint8_t data)
{
    if (timerId == measureTimer) {
        measureTimer = 0;
    }
}

void BatteryMonitor::StopTimer()
{
    if (measureTimer != 0) {
        Timer::Stop(measureTimer);
        measureTimer = 0;
    }
}
void BatteryMonitor::StartTimer()
{
    StopTimer();
    measureTimer = Timer::Start(this, DELAY_BETWEEN_MEASURE);
}

float BatteryMonitor::GetStateVoltage(VoltageLevelState state)
{
	return ((float)valueLevel[(uint8_t)state]) / 10.0f;
}

void BatteryMonitor::GetBatteryData(BatteryData* data) {
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage
    if (data == NULL) return;

    // TODO:
    data->voltage = voltMeter.Voltage();
	data->pinVoltage = voltMeter.PinVoltage();
    data->ampers = amperMeter.Ampers();
    data->calcVoltage = CalcVoltage();
}

double BatteryMonitor::CalcVoltage()
{
    double calculated = voltMeter.Voltage() + (amperMeter.Ampers() * 0.1);

    if (calculated >= GetStateVoltage(VoltageLevelState::NORMAL_LEVEL)) {
        return voltMeter.Voltage();
    }
    return calculated;
}
