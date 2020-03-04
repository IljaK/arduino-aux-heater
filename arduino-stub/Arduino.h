#pragma once
#include <stdint.h>
#include <cstdio>
#include <string>
#include <ctime>
#include "Stream.h"

#include <cstdlib>
#include <climits>

#define MAXBYTE UCHAR_MAX

#define LOW 0x0
#define HIGH 0x1

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2
#define LED_BUILTIN 13

#define PIN_A0   (14)
#define PIN_A1   (15)
#define PIN_A2   (16)
#define PIN_A3   (17)
#define PIN_A4   (18)
#define PIN_A5   (19)
#define PIN_A6   (20)
#define PIN_A7   (21)

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
extern uint16_t analogValues[];
extern int strcasecmp(const char *str1,const char *str2);
extern uint16_t voltageToPinValue(double r1, double r2, double voltage, double vcc);

extern char *dtostrf(double __val, signed char __width, unsigned char __prec, char *__s);