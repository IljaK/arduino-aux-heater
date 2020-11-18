#include <string.h>
#include "BLEHandler.h"


BLEHandler::BLEHandler(BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB)
{
    this->bme280DataCB = bme280DataCB;
    this->batteryDataCB = batteryDataCB;
    debugPrint = &bleServerHandler;
}

void BLEHandler::OnTimerComplete(TimerID timerId)
{
    /*
    if (timerId == statsTimer) {
        statsTimer = 0;
        if (pServer != NULL) SendStats();
    }
    */
}

void BLEHandler::SendStats()
{
    if (statsTimer != 0) return;
    statsTimer = Timer::Start(this, STATS_REFRESH_RATE);

    if (bme280DataCB != NULL) {
        BME280Data bme280Data;
        bme280DataCB(&bme280Data);
        bleServerHandler.SendData(bme280Data);
    }

    if (batteryDataCB != NULL) {
        BatteryData batteryData;
        batteryDataCB(&batteryData);
        bleServerHandler.SendData(batteryData);
    }

    DeviceSpecData deviceData;
    deviceData.remainRam = remainRam();

    bleServerHandler.SendData(deviceData);
}

void BLEHandler::Loop()
{
    bleServerHandler.Loop();
}

void BLEHandler::Start()
{
    bleServerHandler.Start();
}
