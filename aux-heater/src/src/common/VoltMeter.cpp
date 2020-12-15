#include "VoltMeter.h"
#include "Util.h"

#if ESP32 || ARDUINO_ARCH_SAMD
#define MEASURE_BIT_SIZE 4096 // 12 bit size
#else
#define MEASURE_BIT_SIZE 1024 // 10 bit size
#endif

constexpr double MEASURMENTS_RANGE = MEASURE_BIT_SIZE - 1;

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

void VoltMeter::OnTimerComplete(TimerID timerId, uint8_t data)
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
		default:
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
	double vcc = ReadVCC() / 1000.0;
	uint16_t analog_value = analogRead(VOLTMETER_MEASURE_PIN);
	pinValue = (float)(((double)analog_value * vcc * 1.016) / MEASURMENTS_RANGE);
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
#if ARDUINO_TEST 
	return 5000;
#elif ESP32 || ARDUINO_ARCH_SAMD
	return 3300;
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

