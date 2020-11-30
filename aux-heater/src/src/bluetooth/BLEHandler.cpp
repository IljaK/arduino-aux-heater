#include <string.h>
#include "BLEHandler.h"


BLEHandler::BLEHandler(BinaryMessageCallback rxCallback, BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB)
{
    this->rxCallback = rxCallback;
    this->bme280DataCB = bme280DataCB;
    this->batteryDataCB = batteryDataCB;
    debugPrint = this;
}

BLEHandler::~BLEHandler()
{
}

void BLEHandler::OnTimerComplete(TimerID timerId, uint8_t data)
{
    if (timerId == statsTimer) {
        statsTimer = 0;
        SendStats();
    }
}

void BLEHandler::SendStats()
{
    if (statsTimer != 0) return;
    StartStatsTimer();

    if (bme280DataCB != NULL) {
        BME280Data bme280Data;
        bme280DataCB(&bme280Data);
        BLESerialHandler::SendData(bme280Data);
    }

    if (batteryDataCB != NULL) {
        BatteryData batteryData;
        batteryDataCB(&batteryData);
        BLESerialHandler::SendData(batteryData);
    }

    DeviceSpecData deviceData;
    deviceData.remainRam = remainRam();
    deviceData.activeTime = time(NULL) - startTime;

    BLESerialHandler::SendData(deviceData);
}

void BLEHandler::Loop()
{
    BLESerialHandler::Loop();
    if (AvailableMessages() > 0) {
        BinaryMessage * message = GetMessage();
        if (message != NULL) {
            if (rxCallback != NULL) {
                rxCallback(message);
            }
            if (message->data != NULL) {
                free(message->data);
            }
            free(message);
        }
    }
}

void BLEHandler::Start()
{
    BLESerialHandler::Start();
}

void BLEHandler::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param)
{
    // May launch from different core!
    BLESerialHandler::onConnect(pServer, param);
    StartStatsTimer();
}

void BLEHandler::onDisconnect(BLEServer* pServer)
{
    // May launch from different core!
    BLESerialHandler::onDisconnect(pServer);
    if (GetConnectedCount() <= 1) {
        StopStatsTimer();
    }
}

void BLEHandler::StartStatsTimer()
{
    if (statsTimer == 0) {
        statsTimer = Timer::Start(this, STATS_REFRESH_RATE);
    }
}

void BLEHandler::StopStatsTimer()
{
    if (statsTimer != 0) {
        Timer::Stop(statsTimer);
        statsTimer = 0;
    }
}
