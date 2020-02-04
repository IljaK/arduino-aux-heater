#include "VoltMeter.h"
#include "Util.h"


VoltMeter::VoltMeter(float r1, float r2)
{
	this->r1 = r1;
	this->r2 = r2;

	pinMode(VOLTMETER_MEASURE_PIN, INPUT);
	pinMode(VOLTMETER_TRIGGER_PIN, OUTPUT);
	digitalWrite(VOLTMETER_TRIGGER_PIN, LOW);

	measureState = VoltMeterState::WAIT_NEXT;
}


VoltMeter::~VoltMeter()
{
}

void VoltMeter::Loop()
{
	timer.Loop();

	if (!timer.IsRunning())
	{
		switch (measureState) {
		case VoltMeterState::WAIT_NEXT:
			measureState = VoltMeterState::MEASURE;
			digitalWrite(VOLTMETER_TRIGGER_PIN, HIGH);
			timer.Start(DELAY_BEFORE_MEASURE);
			break;
		case VoltMeterState::MEASURE:
			digitalWrite(VOLTMETER_TRIGGER_PIN, LOW);
			StartMeasureTimer(DELAY_BETWEEN_MEASURE);
			MeasureVoltage();
			OnVoltageMeasured();
			break;
		}
	}
}

void VoltMeter::OnVoltageMeasured()
{

}
void VoltMeter::StartMeasureTimer(uint32_t delay)
{
	measureState = VoltMeterState::WAIT_NEXT;
	timer.Start(delay);
}

void VoltMeter::MeasureVoltage()
{
	long vcc = ReadVCC();
	outPrintf("VCC: %d", (int)(vcc));
	int analog_value = analogRead(VOLTMETER_MEASURE_PIN);
	pinValue = (float)(((double)analog_value * (double)vcc) / 1024000.0);
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

