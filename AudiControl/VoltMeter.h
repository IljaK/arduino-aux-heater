#pragma once
#include "Timer.h"

constexpr uint32_t DELAY_BEFORE_MEASURE = 1000u;
constexpr uint32_t DELAY_BETWEEN_MEASURE = 30000000u; //60000000u; // 60 sec
constexpr uint32_t DELAY_ATTEMPT_MEASURE = 20000000u;

enum class VoltMeterState : uint8_t
{
	NONE,
	WAIT_NEXT,
	MEASURE
};

class VoltMeter
{
private:

	float r1;
	float r2;
	float pinValue = 0;
	Timer timer;

	VoltMeterState measureState;

	void MeasureVoltage();
	long ReadVCC();

protected:
	virtual void OnVoltageMeasured();
	void StartMeasureTimer(uint32_t delay);

public:
	VoltMeter(float r1, float r2);
	~VoltMeter();

	void Loop();

	float Voltage();
	float PinVoltage();

	float R1() { return r1; }
	float R2() { return r2; }
};

