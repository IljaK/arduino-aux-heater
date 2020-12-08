#if ESP32
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
#include "BLESerialHandler.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define STATS_REFRESH_RATE 1000000u

class BLEHandler : public BLESerialHandler, public DebugHandler, public ITimerCallback
{
private:

	BME1280DataCallback bme280DataCB = NULL;
    BatteryDataCallback batteryDataCB = NULL;
    BinaryMessageCallback rxCallback = NULL;
    TimerID statsTimer = 0;

protected:

    // BLE Server callbacks
	void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override;
    void onDisconnect(BLEServer* pServer) override;

public:
    BLEHandler(BinaryMessageCallback rxCallback, BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB);
    virtual ~BLEHandler();

	void OnTimerComplete(TimerID timerId, uint8_t data);

    void StopStatsTimer();
    void StartStatsTimer();

    void SendStats();

    void Start() override;
    void Loop() override;
};
#endif