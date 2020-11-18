#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "../common/DebugHandler.h"
#include "../common/Timer.h"
#include "../common/Util.h"
#include "../common/ByteStackArray.h"
#include "BLEServerHandler.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

#define UART_TX_CHARACTERISTICS "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_RX_CHARACTERISTICS "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define BME280_STATE_UUID "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
#define BATTERY_STATE_UUID "6E400005-B5A3-F393-E0A9-E50E24DCCA9E"
#define MEMORY_STATE_UUID "6E400006-B5A3-F393-E0A9-E50E24DCCA9E"


#define MAX_BLE_MESSAGE_SIZE 20u
#define MAX_TX_STACK_SIZE 20u

#define MAX_CONNECTED_DEVICES 3u
#define STATS_REFRESH_RATE 1000000u

class BLEHandler : public DebugHandler, public ITimerCallback
{
private:

    BLEServerHandler bleServerHandler;

	BME1280DataCallback bme280DataCB = NULL;
    BatteryDataCallback batteryDataCB = NULL;
    TimerID statsTimer = 0;
    void SendStats();

public:
    BLEHandler(BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB);
	void OnTimerComplete(TimerID timerId);

public:

    void Start();
    void Loop();
};