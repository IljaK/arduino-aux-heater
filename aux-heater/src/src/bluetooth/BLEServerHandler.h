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
#define MAX_TX_STACK_SIZE 20u
#define MAX_RX_STACK_SIZE 3u

#define MAX_CONNECTED_DEVICES 3u
#define STATS_REFRESH_RATE 1000000u

#define PACKAGE_EXTRA_DATA_SIZE 1
constexpr uint8_t MAX_BLE_PART_SIZE = MAX_BLE_MESSAGE_SIZE - PACKAGE_EXTRA_DATA_SIZE;


struct BLEMessage {
    uint8_t length;
    uint8_t data[];
};

class BLEServerHandler : 
    public Print,
    private BLEServerCallbacks, 
    private BLECharacteristicCallbacks, 
    private BLESecurityCallbacks, 
    private BLEDescriptorCallbacks
{
private:
    BLEServer* pServer = NULL;
    BLEService* pService = NULL;
    BLESecurity *pSecurity = NULL;

    BLECharacteristic* uartRXCharacteristics;
    BLECharacteristic* uartTXCharacteristics;

    BLECharacteristic* batteryCharacteristics;
    BLECharacteristic* bme280Characteristics;
    BLECharacteristic* deviceCharacteristics;

    BLECharacteristic* CreateCharacteristic(const char * uuid, uint32_t properties);

    // Serial TX
    ByteStackArray * serialTXBuffer = new ByteStackArray(MAX_TX_STACK_SIZE, MAX_BLE_MESSAGE_SIZE);
    bool isTransferrig = false;

    // Serial RX
    BinaryMessageStack * rxMessageStack = new BinaryMessageStack(MAX_RX_STACK_SIZE);

    void SendSerialMessage();

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

    void Advertise();
    void StopAdvertise();
    void ReadRxData(uint8_t * data);
    //bool IsValidCheckSum(BLEMessage * message);
public:
    BLEServerHandler();
    virtual ~BLEServerHandler();

    void Start();
    void Loop();
    void SendSerial(uint8_t * data, uint8_t length);

    void SendData(BME280Data bme280Data);
    void SendData(BatteryData batteryData);
    void SendData(DeviceSpecData deviceData);

    // Compatibility with serial->write
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;    
};