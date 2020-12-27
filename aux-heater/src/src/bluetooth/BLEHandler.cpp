#if ESP32
#include <string.h>
#include "BLEHandler.h"

BLEHandler::BLEHandler(BinaryMessageCallback rxCallback, BME1280DataCallback temperatureDataCB, BatteryDataCallback batteryDataCB)
{
    this->rxCallback = rxCallback;
    this->temperatureDataCB = temperatureDataCB;
    this->batteryDataCB = batteryDataCB;
    debugPrint = this;
}

BLEHandler::~BLEHandler()
{
}

void BLEHandler::SendStats(TemperatureData * temperatureData, BatteryData * batteryData)
{
    if (temperatureData != NULL) {
        BLESerialHandler::SendData(temperatureData);
    }

    if (batteryData != NULL) {
        BLESerialHandler::SendData(batteryData);
    }

    DeviceSpecData deviceData;
    deviceData.remainRam = remainRam();
    deviceData.activeTime = TimeManager::GetOnlineSeconds();

    BLESerialHandler::SendData(deviceData);
}

void BLEHandler::Loop()
{
    BLESerialHandler::Loop();
    if (AvailableMessages() > 0) {
        BinaryMessage * message = GetMessage();
        if (message != NULL) {
            if (rxCallback != NULL) {
                rxCallback((char *)message->data, (size_t)message->length);
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
    isDebugEnabled = true;
}

void BLEHandler::onDisconnect(BLEServer* pServer)
{
    // May launch from different core!
    BLESerialHandler::onDisconnect(pServer);
    isDebugEnabled = false;
}

#endif
