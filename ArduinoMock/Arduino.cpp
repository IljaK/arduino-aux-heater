#include <Windows.h>
#include <chrono>
#include <stdint.h>

using namespace std::chrono;
steady_clock::time_point launchTime = steady_clock::now();
unsigned long timeOffset = 0;
uint16_t analogValues[21];

unsigned long micros()
{
	//auto now = steady_clock::now();
	//long long count = duration_cast<microseconds>(now - launchTime).count();
	//return (unsigned long)count + timeOffset;
	return timeOffset;
}

unsigned long millis()
{
	return micros() * 1000ul;
}

void delay(unsigned long milliseconds)
{
	Sleep(milliseconds);
}
void delayMicroseconds(unsigned int us)
{
	Sleep((unsigned long)us / 1000ul);
}

void pinMode(uint8_t, uint8_t)
{

}
void digitalWrite(uint8_t, uint8_t)
{

}
int digitalRead(uint8_t pin)
{
	return analogValues[pin];
}
int analogRead(uint8_t pin)
{
	return analogValues[pin];
}
void analogReference(uint8_t mode)
{

}
void analogWrite(uint8_t, int)
{

}

int strcasecmp(const char *str1, const char *str2)
{
	int ca, cb;
	do {
		ca = (unsigned char)*str1++;
		cb = (unsigned char)*str2++;
		ca = tolower(ca);
		cb = tolower(cb);
	} while (ca == cb && ca != '\0');
	return ca - cb;
}

uint16_t voltageToPinValue(double r1, double r2, double voltage, double vcc) {
	double pinValue = (voltage * (r2 / (r1 + r2))) * 1024.0 / vcc;
	return (uint16_t)pinValue;
}

char *dtostrf(double __val, signed char __width, unsigned char __prec, char *__s)
{
	// TODO:
	return __s;
}