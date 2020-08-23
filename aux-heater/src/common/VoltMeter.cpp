#include "VoltMeter.h"
#include "Util.h"

VoltMeter::VoltMeter(float r1, float r2):ITimerCallback()
{
	this->r1 = r1;
	this->r2 = r2;

	pinMode(VOLTMETER_MEASURE_PIN, INPUT);
	pinMode(VOLTMETER_TRIGGER_PIN, OUTPUT);
	digitalWrite(VOLTMETER_TRIGGER_PIN, LOW);

	StartMeasureTimer(VoltMeterState::WAIT_NEXT, DELAY_BETWEEN_MEASURE);
}


VoltMeter::~VoltMeter()
{
	if (timer != 0) Timer::Stop(timer);
}

void VoltMeter::OnTimerComplete(TimerID timerId)
{
	if (timerId == timer) {
		switch (measureState) {
		case VoltMeterState::WAIT_NEXT:
			digitalWrite(VOLTMETER_TRIGGER_PIN, HIGH);
			StartMeasureTimer(VoltMeterState::MEASURE, DELAY_BEFORE_MEASURE);
			break;
		case VoltMeterState::MEASURE:
			StartMeasureTimer(VoltMeterState::WAIT_NEXT, DELAY_BETWEEN_MEASURE);
			MeasureVoltage();
			OnVoltageMeasured();
			digitalWrite(VOLTMETER_TRIGGER_PIN, LOW);
			break;
		}
	}
}

void VoltMeter::OnVoltageMeasured()
{

}
void VoltMeter::StartMeasureTimer(VoltMeterState state, uint32_t delay)
{
	measureState = state;
	if (timer != 0) Timer::Stop(timer);
	timer = Timer::Start(this, delay);
}

void VoltMeter::MeasureVoltage()
{
	long vcc = ReadVCC();
	int analog_value = analogRead(VOLTMETER_MEASURE_PIN);
	outPrintf("VCC: %d", (int)(vcc));
	outPrintf("Analog Val: %d", analog_value);
	pinValue = (float)(((double)analog_value * (double)vcc * (double)1.018f) / 1023000.0);
}


float VoltMeter::PinVoltage()
{
	return pinValue;
}

float VoltMeter::Voltage()
{
	double voltage = (double)pinValue / ((double)r2 / ((double)r1 + (double)r2));

	if (voltage < 0.1) voltage = 0.0;

	return (float)voltage;
}

long VoltMeter::ReadVCC() {
#ifdef ARDUINO_TEST
	return 5000;
#else
	long result; // Read 1.1V reference against AVcc 
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delay(2); // Wait for Vref to settle 
	ADCSRA |= _BV(ADSC); // Convert 
	while (bit_is_set(ADCSRA,ADSC)); 
	result = ADCL; 
	result |= ADCH<<8; 
	result = 1126400L / result; // Back-calculate AVcc in mV
	return result; 
#endif
}

