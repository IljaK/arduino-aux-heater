#pragma once
#include <Arduino.h>

#ifndef MKRGSM1400
#define MKRGSM1400
#endif

#if defined(ESP32)
	#define AUX_RX_PIN 9
	#define AUX_TX_PIN 10

	#define GSM_RX_PIN 16
	#define GSM_TX_PIN 17

	#define VOLTMETER_MEASURE_PIN A4
	#define AMPERMETER_MEASURE_PIN A5
	#define VOLTMETER_TRIGGER_PIN 5
#elif defined(MKRGSM1400)
	#define BT_RX_PIN 1
	#define BT_TX_PIN 0

	#define AUX_RX_PIN 13
	#define AUX_TX_PIN 14

	#define VOLTMETER_MEASURE_PIN A4
	#define AMPERMETER_MEASURE_PIN A5
	#define VOLTMETER_TRIGGER_PIN 5

    #define DS18S20_PIN 2
    #define BT_BAUD_RATE 115200

    #define BT_PIN_ENABLE 3
    #define BT_STATE_PIN 4
#endif

#define ACC_STATE_PIN 5
#define EMERGENCY_STATE_PIN 6
#define SERVICE_STATE_PIN 7

#define AUX_BAUD_RATE 2400
#define SERIAL_BAUD_RATE 115200