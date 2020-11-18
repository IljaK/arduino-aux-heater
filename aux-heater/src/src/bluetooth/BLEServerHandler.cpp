#include <string.h>
#include "BLEServerHandler.h"


BLEServerHandler::BLEServerHandler()
{
}

BLEServerHandler::~BLEServerHandler()
{
}

void BLEServerHandler::Start()
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
    //uartRXCharacteristics = CreateCharacteristic(
    //    UART_RX_CHARACTERISTICS,
    //    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_INDICATE
    //);
    //uartRXCharacteristics->setCallbacks(this);

    uartRXCharacteristics = pService->createCharacteristic(
        UART_RX_CHARACTERISTICS, 
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_INDICATE);

    BLE2902 * descriptor = new BLE2902();
    descriptor->setIndications(true);
    descriptor->setCallbacks(this);
    uartRXCharacteristics->addDescriptor(descriptor);
    uartRXCharacteristics->setCallbacks(this);

    uartTXCharacteristics = CreateCharacteristic(
        UART_TX_CHARACTERISTICS,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    // Battery Characteristic
    batteryCharacteristics = CreateCharacteristic(
        BATTERY_STATE_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    // Bme 280 Characteristic
    bme280Characteristics = CreateCharacteristic(
        BME280_STATE_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    deviceCharacteristics = CreateCharacteristic(
        MEMORY_STATE_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
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

BLECharacteristic * BLEServerHandler::CreateCharacteristic(const char * uuid, uint32_t properties)
{
    BLECharacteristic * characteristic = pService->createCharacteristic(uuid, properties);

    BLE2902 * descriptor = new BLE2902();
    descriptor->setNotifications(true);
    descriptor->setCallbacks(this);

    characteristic->addDescriptor(descriptor);
    characteristic->setCallbacks(this);

    return characteristic;
}

void BLEServerHandler::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param)
{
    Serial.println("onConnect");

    uartTXCharacteristics->setValue((uint8_t *)"test value", 11);
    uartTXCharacteristics->indicate();
}

void BLEServerHandler::onDisconnect(BLEServer* pServer)
{
    Serial.println("onDisconnect");
    BinaryMessage * msg = rxMessageStack->PeekLast();
    if (msg != NULL && !msg->isFilled()) {
        msg = rxMessageStack->UnshiftLast();
        free(msg->data);
        free(msg);
    }
}

// BLECharacteristicCallbacks
void BLEServerHandler::onRead(BLECharacteristic* pCharacteristic)
{
    Serial.print("onRead chrst: ");
    Serial.println(pCharacteristic->getUUID().toString().data());
}

void BLEServerHandler::onNotify(BLECharacteristic* pCharacteristic)
{
    Serial.print("onNotify chrst: ");
    Serial.println(pCharacteristic->getUUID().toString().data());
    Serial.print("core: ");
    Serial.println(xPortGetCoreID());
}

void BLEServerHandler::onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code)
{
    Serial.print("onStatus chrst: ");
    Serial.println(pCharacteristic->getUUID().toString().data());

    Serial.print("status: ");
    Serial.print(s);
    Serial.print(" code: ");
    Serial.print(code);
}

void BLEServerHandler::onWrite(BLECharacteristic* pCharacteristic) {

    Serial.print("onWrite chrst: ");
    Serial.println(pCharacteristic->getUUID().toString().data());
    
    if (pCharacteristic == uartRXCharacteristics) {
        ReadRxData(pCharacteristic->getData());
    }
}
void BLEServerHandler::ReadRxData(uint8_t * data)
{
    BLEMessage * bleMessage = (BLEMessage *)data;
    BinaryMessage * msg = rxMessageStack->PeekLast();
    if (msg != NULL) {
        if (rxMessageStack->IsFull() && msg->isFilled()) {
            return;
        }
    }
    uint8_t size = 0;
    if (msg == NULL) {
        Serial.print("New msg: ");
        Serial.println(bleMessage->length);

        msg = (BinaryMessage *)malloc(sizeof(BinaryMessage));
        msg->length = bleMessage->length;
        msg->data = (uint8_t *)malloc(msg->length);
        msg->filled = 0;

        rxMessageStack->Append(msg);
    }

    if (bleMessage->length > MAX_BLE_PART_SIZE) {
        size = MAX_BLE_PART_SIZE;
    } else {
        size = bleMessage->length;
    }

    if (msg->filled + size > msg->length) {
        return;
    }
    memcpy(msg->data + msg->filled, (const uint8_t *)bleMessage->data, size);
    msg->filled += size;

    if (msg->isFilled()) {
        Serial.print((char *)msg->data);
        Serial.println();

        BinaryMessage * delMsg = rxMessageStack->UnshiftFirst();
        free(delMsg->data);
        free(delMsg);
    }
}

void BLEServerHandler::onRead(BLEDescriptor* pDescriptor)
{
    Serial.print("onRead descriptor: ");
    Serial.println(pDescriptor->getUUID().toString().data());
}

void BLEServerHandler::onWrite(BLEDescriptor* pDescriptor)
{
    Serial.print("onWrite descriptor: ");
    Serial.println(pDescriptor->getUUID().toString().data());
}

uint32_t BLEServerHandler::onPassKeyRequest()
{
    return 123456;
}

void BLEServerHandler::onPassKeyNotify(uint32_t pass_key)
{
    Serial.printf("The passkey Notify number: %d\n", pass_key);
}

bool BLEServerHandler::onSecurityRequest()
{
    return true;
}

void BLEServerHandler::onAuthenticationComplete(esp_ble_auth_cmpl_t)
{
    Serial.println("onAuthenticationComplete!");
}

bool BLEServerHandler::onConfirmPIN(uint32_t pin)
{
    Serial.println("onConfirmPIN!");
    return true;
}

void BLEServerHandler::SendSerial(uint8_t * data, uint8_t length)
{
    if (pServer->getConnectedCount() == 0) {
        return;
    }

    serialTXBuffer->Append(data, length);
    if (serialTXBuffer->HasFilledPacket()) {
        SendSerialMessage();
    }
}

void BLEServerHandler::Loop()
{
    if (serialTXBuffer->HasFilledPacket()) {
        SendSerialMessage();
    }
}

void BLEServerHandler::SendSerialMessage()
{
    if (!isTransferrig && serialTXBuffer->Size() > 0) {
        ByteArray * item = serialTXBuffer->UnshiftFirst();
        Serial.printf("SendSerialMessage: %u\r\n", item->length);

        uartTXCharacteristics->setValue(item->array, item->length);
        uartTXCharacteristics->notify(false);

        free(item->array);
        free(item);
    }
}

void BLEServerHandler::Advertise()
{
    if (pServer == NULL) return;
    if (pServer->getAdvertising() == NULL) return;
    pServer->getAdvertising()->start();
}

void BLEServerHandler::StopAdvertise()
{
    if (pServer == NULL) return;
    if (pServer->getAdvertising() == NULL) return;
    pServer->getAdvertising()->stop();
}

void BLEServerHandler::SendData(BME280Data bme280Data)
{
    bme280Characteristics->setValue((uint8_t *)&bme280Data, sizeof(bme280Data));
    bme280Characteristics->notify(true);
}
void BLEServerHandler::SendData(BatteryData batteryData)
{
    batteryCharacteristics->setValue((uint8_t *)&batteryData, sizeof(batteryData));
    batteryCharacteristics->notify(true);
}
void BLEServerHandler::SendData(DeviceSpecData deviceData)
{
    deviceCharacteristics->setValue((uint8_t *)&deviceData, sizeof(deviceData));
    deviceCharacteristics->notify(true);
}

// Compatibility with serial->write
size_t BLEServerHandler::write(uint8_t data)
{
    SendSerial(&data, 1);
    return 1;
}
size_t BLEServerHandler::write(const uint8_t *buffer, size_t size)
{
    SendSerial((uint8_t *)buffer, size);
    return size;
}

/*
bool BLEServerHandler::IsValidCheckSum(BLEMessage * message)
{
    if (message->length == 0) {
        return false;
    }
    
    uint8_t calculatedSum = 0;
    uint8_t length = message->length;

    if (length > MAX_BLE_PART_SIZE) length = MAX_BLE_PART_SIZE;
    
    for (uint8_t i = 0; i < length; i++) {
        calculatedSum += message->data[i];
    }
    return calculatedSum == message->checkSum;
}
*/
