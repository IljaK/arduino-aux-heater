#pragma once

#include <stdint.h>
#ifdef _LINUX_
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <ctype.h>
#include <time.h>

#include <cstdio>
#include <string>
#include <ctime>
#include "Interrupt.h"
#include "EEprom.h"
#include "Stream.h"
#include "Mutex.h"
#include "Sercom.h"
#include "Uart.h"

#include <cstdlib>
#include <climits>

#include "HardwareSerial.h"
/** One hour, expressed in seconds */
#define ONE_HOUR 3600

#define MAXBYTE UCHAR_MAX

typedef uint8_t pin_size_t;
typedef void (*voidFuncPtr)(void);
typedef void (*voidFuncPtrParam)(void*);

typedef enum {
  LOW     = 0,
  HIGH    = 1,
  CHANGE  = 2,
  FALLING = 3,
  RISING  = 4,
} PinStatus;

typedef enum {
  INPUT           = 0x0,
  OUTPUT          = 0x1,
  INPUT_PULLUP    = 0x2,
  INPUT_PULLDOWN  = 0x3,
} PinMode;

#define LED_BUILTIN 13
#define digitalPinToInterrupt(P)   ( P )

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

typedef enum _EPioType
{
  PIO_NOT_A_PIN=-1,     /* Not under control of a peripheral. */
  PIO_EXTINT=0,         /* The pin is controlled by the associated signal of peripheral A. */
  PIO_ANALOG,           /* The pin is controlled by the associated signal of peripheral B. */
  PIO_SERCOM,           /* The pin is controlled by the associated signal of peripheral C. */
  PIO_SERCOM_ALT,       /* The pin is controlled by the associated signal of peripheral D. */
  PIO_TIMER,            /* The pin is controlled by the associated signal of peripheral E. */
  PIO_TIMER_ALT,        /* The pin is controlled by the associated signal of peripheral F. */
  PIO_COM,              /* The pin is controlled by the associated signal of peripheral G. */
  PIO_AC_CLK,           /* The pin is controlled by the associated signal of peripheral H. */
  PIO_DIGITAL,          /* The pin is controlled by PORT. */
  PIO_INPUT,            /* The pin is controlled by PORT and is an input. */
  PIO_INPUT_PULLUP,     /* The pin is controlled by PORT and is an input with internal pull-up resistor enabled. */
  PIO_OUTPUT,           /* The pin is controlled by PORT and is an output. */

  PIO_PWM=PIO_TIMER,
  PIO_PWM_ALT=PIO_TIMER_ALT,
} EPioType ;

#define GSM_RTS    (28u)
#define GSM_CTS    (29u)
#define GSM_RESETN (31u)
#define GSM_DTR    (35u)

extern Stream SerialGSM;
extern Stream Serial;
extern Stream Serial1;

extern SERCOM sercom0;
extern SERCOM sercom1;
extern SERCOM sercom2;
extern SERCOM sercom3;
extern SERCOM sercom4;
extern SERCOM sercom5;

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

extern void attachInterrupt(pin_size_t pin, voidFuncPtr callback, PinStatus mode);
extern int pinPeripheral( uint32_t ulPin, EPioType ulPeripheral );
extern void analogReadResolution(int res);
