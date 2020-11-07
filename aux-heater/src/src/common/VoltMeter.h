#pragma once
#include "Timer.h"

constexpr uint32_t DELAY_BEFORE_MEASURE = 2000u;
constexpr uint32_t DELAY_BETWEEN_MEASURE = 1000000u; //60000000u; // 60 sec
constexpr uint32_t DELAY_ATTEMPT_MEASURE = 1000000u;

enum class VoltMeterState : uint8_t
{
	NONE,
	WAIT_NEXT,
	MEASURE
};

class VoltMeter: public ITimerCallback
{
private:

	float r1;
	float r2;
	float pinValue = 0;
	TimerID timer = 0;

	VoltMeterState measureState;

	void MeasureVoltage();
	long ReadVCC();

protected:
	virtual void OnVoltageMeasured();
	void StartMeasureTimer(VoltMeterState state, uint32_t delay);

public:
	VoltMeter(float r1, float r2);
	~VoltMeter();

	void OnTimerComplete(TimerID timerId) override;

	float Voltage();
	float PinVoltage();

	float R1() { return r1; }
	float R2() { return r2; }
};

