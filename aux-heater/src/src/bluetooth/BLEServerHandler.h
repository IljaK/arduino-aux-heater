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
#include "../common/ByteArrayStream.h"
#include <Stream.h>
#include "../common/BinaryMessageStack.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

#define UART_TX_CHARACTERISTICS "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_RX_CHARACTERISTICS "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define BME280_STATE_UUID "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
#define BATTERY_STATE_UUID "6E400005-B5A3-F393-E0A9-E50E24DCCA9E"
#define MEMORY_STATE_UUID "6E400006-B5A3-F393-E0A9-E50E24DCCA9E"


#define MAX_BLE_MESSAGE_SIZE 20u

#define MAX_CONNECTED_DEVICES 3u
#define STATS_REFRESH_RATE 1000000u

class BLEServerHandler :
    protected BLEServerCallbacks, 
    protected BLECharacteristicCallbacks, 
    protected BLESecurityCallbacks, 
    protected BLEDescriptorCallbacks
{
private:
    BLEServer* pServer = NULL;
    BLEService* pService = NULL;
    BLESecurity *pSecurity = NULL;

    BLECharacteristic* CreateCharacteristic(const char * uuid, uint32_t properties);

    void Advertise();
    void StopAdvertise();
    //bool IsValidCheckSum(BLEMessage * message);

protected:

    BLECharacteristic* uartRXCharacteristics;
    BLECharacteristic* uartTXCharacteristics;

    BLECharacteristic* batteryCharacteristics;
    BLECharacteristic* bme280Characteristics;
    BLECharacteristic* deviceCharacteristics;

    // BLE Server callbacks
	void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override;
    void onDisconnect(BLEServer* pServer) override;

    // BLECharacteristicCallbacks
    void onRead(BLECharacteristic* pCharacteristic) override;
	void onWrite(BLECharacteristic* pCharacteristic) override;
	void onNotify(BLECharacteristic* pCharacteristic) override;
	void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code) override;

    // Descriptor callbacks
    void onRead(BLEDescriptor* pDescriptor) override;
	void onWrite(BLEDescriptor* pDescriptor) override;

    // Authentication
    uint32_t onPassKeyRequest() override;
	void onPassKeyNotify(uint32_t pass_key) override;
	bool onSecurityRequest()  override;
	void onAuthenticationComplete(esp_ble_auth_cmpl_t) override;
	bool onConfirmPIN(uint32_t pin) override;

public:
    BLEServerHandler();
    virtual ~BLEServerHandler();

    virtual void Start();

    void SendData(BME280Data bme280Data);
    void SendData(BatteryData batteryData);
    void SendData(DeviceSpecData deviceData);

    int GetConnectedCount();
};