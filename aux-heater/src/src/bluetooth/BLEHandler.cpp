#include "BLEHandler.h"

BLEHandler::BLEHandler(BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB)
{
    this->bme280DataCB = bme280DataCB;
    this->batteryDataCB = batteryDataCB;
    debugPrint = this;
}

// Compatibility with serial->write
size_t BLEHandler::write(uint8_t data)
{
    if (serialCharacteristic == NULL || serialRXLength >= MAX_BLE_MESSAGE_SIZE) {
        return 0;
    }

    serialTXBuffer[serialRXLength] = data;
    serialRXLength++;
    if (serialRXLength >= MAX_BLE_MESSAGE_SIZE) {
        SendSerialMessage();
    }
    return 1;
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
    if (connectedDevices == 0) {
        SendStats();
    }
    connectedDevices++;

    if (connectedDevices >= MAX_CONNECTED_DEVICES) {
        StopAdvertise();
    }
    Serial.print("Device connected, total: ");
    Serial.print(connectedDevices);
    Serial.println();
}

void BLEHandler::onDisconnect(BLEServer* pServer) {
    Serial.println("Device disconnected");
    connectedDevices--;
    if (connectedDevices == 0) {
        Advertise();
        Timer::Stop(statsTimer);
        statsTimer = 0;
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
    Serial.print("onNotify chrst: ");
    Serial.println(pCharacteristic->getUUID().toString().data());
}
void BLEHandler::onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code)
{
    Serial.print("onStatus chrst: ");
    Serial.println(pCharacteristic->getUUID().toString().data());
}

void BLEHandler::onWrite(BLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
            Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
    }
}

void BLEHandler::Start()
{
    // Create the BLE Device
    BLEDevice::init("ESP32");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);

    // Create the BLE Service
    BLEService* pService = pServer->createService(SERVICE_UUID);

    //BLE2902 * descriptor = new BLE2902();
    //descriptor->setIndications(true);
    //descriptor->setNotifications(true);

    /*
    // Create a BLE Characteristic
    uartCharacteristics = pService->createCharacteristic(
        UART_TX_CHARACTERISTICS,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );
    uartCharacteristics->addDescriptor(descriptor);
    uartCharacteristics->setCallbacks(this);

    batteryCharacteristics = pService->createCharacteristic(
        BATTERY_STATE_UUID,
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );

    batteryCharacteristics->addDescriptor(descriptor);
    batteryCharacteristics->setCallbacks(this);
    */

    bme280Characteristics = pService->createCharacteristic(
        BME280_STATE_UUID,
        BLECharacteristic::PROPERTY_READ  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );

    bme280Characteristics->setCallbacks(this);


    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting a client connection to notify...");
}

void BLEHandler::OnTimerComplete(TimerID timerId)
{
    if (timerId == statsTimer) {
        statsTimer = 0;
        if (pServer != NULL) SendStats();
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

    //batteryCharacteristics->setValue((uint8_t *)&batteryData, sizeof(batteryData));
    //batteryCharacteristics->notify();

    //bme280Characteristics->getDescriptorByUUID()
    //ble2902->setValue((uint8_t *)&bme280Data, sizeof(bme280Data));
    bme280Characteristics->setValue((uint8_t *)&bme280Data, sizeof(bme280Data));
    bme280Characteristics->notify(true);
    
        // TODO: read voltage
    //outWrite(temperature, 5, 2); // out temp
    /*
    serial->write("0.0");
    serial->write('|');
    writeDouble(serial, statsData.voltage, 5, 2); // in temp
    serial->write('|');
    writeDouble(serial, statsData.ampers, 5, 2); // humidity
    serial->write('|');
    writeDouble(serial, statsData.inPressure, 6, 1); // pressure
    serial->write('|');
    writeDouble(serial, statsData.voltage, 5, 2); // voltage
    serial->write('|');
    writeDouble(serial, statsData.ampers, 5, 2); // ampers
    serial->write('|');
    writeDouble(serial, statsData.calcVoltage, 5, 2); // calculated voltage
    serial->write('|');
    writeASCII(serial, remainRam()); // remain memory
    outWriteEnd();
    */
    
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