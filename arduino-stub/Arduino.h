#pragma once
#include <stdint.h>
#include <cstdio>
#include <string>
#include <ctime>
#include "Interrupt.h"
#include "EEprom.h"
#include "Stream.h"
#include "Mutex.h"

#include <cstdlib>
#include <climits>

#include <time.h>
/** One hour, expressed in seconds */
#define ONE_HOUR 3600

#define MAXBYTE UCHAR_MAX

#define LOW 0x0
#define HIGH 0x1

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2
#define LED_BUILTIN 13


static uint8_t SPCR = 0;
static uint8_t SPE = 0;
static uint8_t MSTR = 0;
static uint8_t SPSR = 0;
static uint8_t SPDR = 0;
static uint8_t SPIF = 0;

// A6-A11 share with digital pins
static const uint8_t A0 = 18;
static const uint8_t A1 = 19;
static const uint8_t A2 = 20;
static const uint8_t A3 = 21;
static const uint8_t A4 = 22;
static const uint8_t A5 = 23;
static const uint8_t A6 = 24;	// D4
static const uint8_t A7 = 25;	// D6
static const uint8_t A8 = 26;	// D8
static const uint8_t A9 = 27;	// D9
static const uint8_t A10 = 28;	// D10
static const uint8_t A11 = 29;	// D12

extern unsigned long millis();
extern unsigned long micros();
extern void delay(unsigned long milliseconds);
extern void delayMicroseconds(unsigned int us);

extern void pinMode(uint8_t, uint8_t);
extern void digitalWrite(uint8_t, uint8_t);
extern int digitalRead(uint8_t);
extern int analogRead(uint8_t);
extern void analogReference(uint8_t mode);
extern void analogWrite(uint8_t, int);

extern unsigned long timeOffset;
extern time_t systemTime;
extern uint16_t analogValues[];

#ifndef _LINUX_
extern int strcasecmp(const char *str1,const char *str2);
#endif

extern uint16_t voltageToPinValue(double r1, double r2, double voltage, double vcc);

extern char *dtostrf(double __val, signed char __width, unsigned char __prec, char *__s);

extern void set_system_time(time_t timestamp);
extern time_t mk_gmtime(struct tm * timeptr);
extern void system_tick(void);
extern void set_zone(int32_t);

extern void reti();

extern char *itoa (int val, char *s, int radix);
extern char *utoa (long val, char *s, int radix);
extern char *ltoa (unsigned int val, char *s, int radix);
extern char *ultoa (unsigned long val, char *s, int radix);
