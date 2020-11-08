#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "../common/DebugHandler.h"
#include "../common/Timer.h"
#include "../common/Util.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

#define UART_TX_CHARACTERISTICS "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_RX_CHARACTERISTICS "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define BME280_STATE_UUID "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
#define BATTERY_STATE_UUID "6E400005-B5A3-F393-E0A9-E50E24DCCA9E"


#define MAX_BLE_MESSAGE_SIZE 20

#define MAX_CONNECTED_DEVICES 3
#define STATS_REFRESH_RATE 1000000

class BLEHandler : public BLEServerCallbacks, public BLECharacteristicCallbacks, public Print, public DebugHandler, public ITimerCallback
{
private:
    BLEServer* pServer = NULL;
    BLECharacteristic* uartRXCharacteristics;
    BLECharacteristic* uartTXCharacteristics;
    BLECharacteristic* batteryCharacteristics;
    BLECharacteristic* bme280Characteristics;

	BME1280DataCallback bme280DataCB = NULL;
    BatteryDataCallback batteryDataCB = NULL;

    uint8_t serialRXLength = 0;
    uint8_t serialTXBuffer[MAX_BLE_MESSAGE_SIZE];

    BLECharacteristic * serialCharacteristic = NULL;
    TimerID statsTimer = 0;

    void SendSerialMessage();
public:
    BLEHandler(BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB);
    // Compatibility with serial->write
    size_t write(uint8_t) override;

    // BLE Server callbacks
	void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param);
    void onDisconnect(BLEServer* pServer);

    // BLECharacteristicCallbacks
    void onRead(BLECharacteristic* pCharacteristic);
	void onWrite(BLECharacteristic* pCharacteristic);
	void onNotify(BLECharacteristic* pCharacteristic);
	void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code);

	void OnTimerComplete(TimerID timerId);
    void SendStats();

public:

    void Start();
    void Loop();

    void Advertise();
    void StopAdvertise();
};