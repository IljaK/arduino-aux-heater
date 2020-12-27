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

class BLEHandler : public BLESerialHandler, public DebugHandler
{
private:
    StringCallback rxCallback = NULL;

protected:

    // BLE Server callbacks
	void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override;
    void onDisconnect(BLEServer* pServer) override;

public:
    BLEHandler(StringCallback rxCallback);
    virtual ~BLEHandler();

    void SendStats(TemperatureData * temperatureData, BatteryData * batteryData);

    void Start() override;
    void Loop() override;
};
#endif