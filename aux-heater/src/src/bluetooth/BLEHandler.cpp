#include "BLEHandler.h"
#include <string.h>

BLEHandler::BLEHandler(BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB)
{
    this->bme280DataCB = bme280DataCB;
    this->batteryDataCB = batteryDataCB;
    debugPrint = this;
}

uint32_t BLEHandler::onPassKeyRequest()
{
    return 123456;
}

void BLEHandler::onPassKeyNotify(uint32_t pass_key)
{
    Serial.printf("The passkey Notify number: %d\n", pass_key);
}

bool BLEHandler::onSecurityRequest()
{
    return true;
}

void BLEHandler::onAuthenticationComplete(esp_ble_auth_cmpl_t)
{
    Serial.println("onAuthenticationComplete!");
}

bool BLEHandler::onConfirmPIN(uint32_t pin)
{
    Serial.println("onConfirmPIN!");
    return true;
}

// Compatibility with serial->write
size_t BLEHandler::write(uint8_t data)
{
    serialTXBuffer[serialRXLength] = data;
    serialRXLength++;
    if (serialRXLength >= MAX_BLE_MESSAGE_SIZE) {
        SendSerialMessage();
    }
    return 1;
}
size_t BLEHandler::write(const uint8_t *buffer, size_t size)
{
    if (buffer == NULL) return 0;
    if (size == 0) return 0;

    size_t sent = 0;

    while (sent < size) {
        size_t remain = size - sent;
        size_t append = MAX_BLE_MESSAGE_SIZE - serialRXLength;
        if (append < remain) append = remain;

        mempcpy(serialTXBuffer + serialRXLength, buffer + sent, append);
        sent += append;

        if (serialRXLength == MAX_BLE_MESSAGE_SIZE) {
            SendSerialMessage();
        }
    }

    return size;
}

void BLEHandler::SendSerialMessage()
{
    if (serialCharacteristic == NULL) return;
    serialCharacteristic->setValue(serialTXBuffer, serialRXLength);
    serialCharacteristic->notify(false);
    serialRXLength = 0;
    // TODO
}

void BLEHandler::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param)
{
    int connectedCount = pServer->getConnectedCount() + 1;

    if (connectedCount == 1) {
        SendStats();
    }

    Serial.print("Device connected, id: ");
    Serial.print(pServer->getConnId());
    Serial.print(" total: ");
    Serial.print(connectedCount);
    Serial.println();

    if (connectedCount < MAX_CONNECTED_DEVICES) {
        Advertise();
    }
    
    //if (testTimer == 0) {
    //    testTimer = Timer::Start(this, 1000000);
    //}
    //pServer->getConnId()
    //pServer->removePeerDevice(pServer->getConnId())
}

void BLEHandler::onDisconnect(BLEServer* pServer) {
    Serial.println("Device disconnected");

    if (pServer->getConnectedCount() == 0) {
        if (statsTimer != 0) {
            Timer::Stop(statsTimer);
            statsTimer = 0;
        }
    }
}

// BLECharacteristicCallbacks
void BLEHandler::onRead(BLECharacteristic* pCharacteristic)
{
    Serial.print("onRead chrst: ");
    Serial.println(pCharacteristic->getUUID().toString().data());
}
void BLEHandler::onNotify(BLECharacteristic* pCharacteristic)
{
    //Serial.print("onNotify chrst: ");
    //Serial.println(pCharacteristic->getUUID().toString().data());
}
void BLEHandler::onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code)
{
    //Serial.print("onStatus chrst: ");
    //Serial.println(pCharacteristic->getUUID().toString().data());
}

void BLEHandler::onWrite(BLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    Serial.print("onWrite Value: ");
    if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++) {
            Serial.print(rxValue[i]);
        }
    }
    Serial.println();
}

void BLEHandler::onRead(BLEDescriptor* pDescriptor)
{
    Serial.print("onRead descriptor: ");
    Serial.println(pDescriptor->getUUID().toString().data());
}
void BLEHandler::onWrite(BLEDescriptor* pDescriptor)
{
    Serial.print("onWrite descriptor: ");
    Serial.println(pDescriptor->getUUID().toString().data());
}

void BLEHandler::Start()
{
    // Create the BLE Device
    BLEDevice::init("ESP32");
    //BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(this);

    // Create the BLE Server
    pServer = BLEDevice::createServer();

    pServer->setCallbacks(this);

    // Create the BLE Service
    pService = pServer->createService(SERVICE_UUID);

    // Create a UART Characteristic
    uartRXCharacteristics = CreateCharacteristic(
        UART_RX_CHARACTERISTICS,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_INDICATE
    );

    uartTXCharacteristics = CreateCharacteristic(
        UART_TX_CHARACTERISTICS,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_INDICATE
    );

    // Battery Characteristic
    batteryCharacteristics = CreateCharacteristic(
        BATTERY_STATE_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    // Bme 280 Characteristic
    bme280Characteristics = CreateCharacteristic(
        BME280_STATE_UUID,
        BLECharacteristic::PROPERTY_READ  | BLECharacteristic::PROPERTY_NOTIFY
    );

    memoryCharacteristics = CreateCharacteristic(
        MEMORY_STATE_UUID,
        BLECharacteristic::PROPERTY_READ  | BLECharacteristic::PROPERTY_NOTIFY
    );

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();

    //pSecurity = new BLESecurity();
    //pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
    //pSecurity->setCapability(ESP_IO_CAP_OUT);
    //pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

    Serial.println("Waiting a client connection to notify...");
}

BLECharacteristic * BLEHandler::CreateCharacteristic(const char * uuid, uint32_t properties)
{
    BLECharacteristic * characteristic = pService->createCharacteristic(uuid, properties);

    BLE2902 * descriptor = new BLE2902();
    descriptor->setNotifications(true);
    descriptor->setCallbacks(this);

    characteristic->addDescriptor(descriptor);
    characteristic->setCallbacks(this);

    return characteristic;
}

void BLEHandler::OnTimerComplete(TimerID timerId)
{
    if (timerId == statsTimer) {
        statsTimer = 0;
        if (pServer != NULL) SendStats();
    } else if (timerId == testTimer) {
        testTimer = 0;
        printf("This messsage is over 20 characters\r\n");
    }
}

void BLEHandler::SendStats()
{
    if (statsTimer != 0) return;
    statsTimer = Timer::Start(this, STATS_REFRESH_RATE);

    BME280Data bme280Data;
    if (bme280DataCB != NULL) bme280DataCB(&bme280Data);

    BatteryData batteryData;
    if (batteryDataCB != NULL) batteryDataCB(&batteryData);

    bme280Characteristics->setValue((uint8_t *)&bme280Data, sizeof(bme280Data));
    bme280Characteristics->notify(true);
    batteryCharacteristics->setValue((uint8_t *)&batteryData, sizeof(batteryData));
    batteryCharacteristics->notify(true);

    uint32_t ram = remainRam();
    memoryCharacteristics->setValue((uint8_t *)&ram, sizeof(ram));
    memoryCharacteristics->notify(true);   
}

void BLEHandler::Loop()
{
    if (serialRXLength >= MAX_BLE_MESSAGE_SIZE) {
        SendSerialMessage();
    }
}

void BLEHandler::Advertise()
{
    if (pServer == NULL) return;
    if (pServer->getAdvertising() == NULL) return;
    pServer->getAdvertising()->start();
}

void BLEHandler::StopAdvertise()
{
    if (pServer == NULL) return;
    if (pServer->getAdvertising() == NULL) return;
    pServer->getAdvertising()->stop();
}