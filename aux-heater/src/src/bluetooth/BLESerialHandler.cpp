#if ESP32
#include "BLESerialHandler.h"

BLESerialHandler::BLESerialHandler() {

}

BLESerialHandler::~BLESerialHandler() {
    FlushRxData();
}

void BLESerialHandler::onDisconnect(BLEServer* pServer)
{
    FlushRxData();

    BLEServerHandler::onDisconnect(pServer);
}

void BLESerialHandler::onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code)
{
    BLEServerHandler::onStatus(pCharacteristic, s, code);

    if (pCharacteristic == uartTXCharacteristics) {
        isTransferrig = false;
        /*
        switch (s)
        {
            case Status::ERROR_GATT:
            case Status::ERROR_INDICATE_DISABLED:
            case Status::ERROR_NO_CLIENT:
                // TODO: Something...
                // break;
            case Status::ERROR_INDICATE_FAILURE:
            case Status::ERROR_INDICATE_TIMEOUT:
                
               // resend data
               // break;
            case Status::SUCCESS_INDICATE:
            case Status::SUCCESS_NOTIFY:
                // Delete sending fragment if indicating
                isTransferrig = false;
                break;
            
            default:
                break;
        }
        */
    }
}


void BLESerialHandler::onWrite(BLECharacteristic* pCharacteristic) 
{
    BLEServerHandler::onWrite(pCharacteristic);
    if (pCharacteristic == uartRXCharacteristics) {
        ReadRxData(pCharacteristic->getData());
    }
}

void BLESerialHandler::FlushRxData()
{
    xSemaphoreTake( xRXSemaphore, portMAX_DELAY );
    while (rxMessageStack->Size() > 0) {
        BinaryMessage * msg = rxMessageStack->UnshiftFirst();
        if (msg->data != NULL) {
            free(msg->data);
        }
        free(msg);
    }
    xSemaphoreGive(xRXSemaphore);
}

void BLESerialHandler::ReadRxData(uint8_t * data)
{
    xSemaphoreTake( xRXSemaphore, portMAX_DELAY );
    
    BLEMessage * bleMessage = (BLEMessage *)data;
    BinaryMessage * msg = rxMessageStack->PeekLast();
    if (msg != NULL) {
        if (rxMessageStack->IsFull() && msg->isFilled()) {

            xSemaphoreGive(xRXSemaphore);
            return;
        }
    }
    uint8_t size = 0;
    if (msg == NULL) {
        Serial.print("New length: ");
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

    if (msg->filled + size <= msg->length) {
        memcpy(msg->data + msg->filled, (const uint8_t *)bleMessage->data, size);
        msg->filled += size;
    }
    // TODO: else must never happen!

    xSemaphoreGive(xRXSemaphore);

    /*
    msg = rxMessageStack->Peek();

    if (msg != NULL && msg->isFilled()) {
        msg = rxMessageStack->UnshiftFirst();

        //if (serialCallBack != NULL) {
        //    serialCallBack((char *)msg->data, (size_t)msg->length);
        //}

        free(msg->data);
        free(msg);
    }
    */
}


int BLESerialHandler::AvailableMessages()
{
    xSemaphoreTake( xRXSemaphore, portMAX_DELAY );
    int size = rxMessageStack->Size();
    if (size > 0) {
        BinaryMessage * msg = rxMessageStack->PeekLast();
        if (msg == NULL || !msg->isFilled()) {
            size--;
        }
    }
    xSemaphoreGive(xRXSemaphore);

    return size;
}

BinaryMessage * BLESerialHandler::GetMessage()
{
    xSemaphoreTake( xRXSemaphore, portMAX_DELAY );

    BinaryMessage * msg = rxMessageStack->Peek();

    if (msg != NULL && msg->isFilled()) {
        msg = rxMessageStack->UnshiftFirst();
    } else {
        msg = NULL;
    }

    xSemaphoreGive(xRXSemaphore);

    return msg;
}

void BLESerialHandler::SendSerialMessage()
{
	xSemaphoreTakeRecursive( xTXSemaphore, portMAX_DELAY );

    if (!isTransferrig && serialTXBuffer->Size() > 0) {

        Serial.print("SendSerialMessage core: ");
        Serial.print(xPortGetCoreID());
        ByteArray * item = serialTXBuffer->UnshiftFirst();

        if (item != NULL) {

            Serial.print(" length: ");
            Serial.println(item->length);

            isTransferrig = true;
            uartTXCharacteristics->setValue(item->array, item->length);
            uartTXCharacteristics->notify(true);

            free(item->array);
            free(item);
        }
    }

    xSemaphoreGiveRecursive(xTXSemaphore);
}

void BLESerialHandler::AppenSerialStack(uint8_t * data, uint8_t length)
{
    if (GetConnectedCount() == 0) {
        return;
    }

	xSemaphoreTakeRecursive( xTXSemaphore, portMAX_DELAY );

    serialTXBuffer->Append(data, length);
    if (serialTXBuffer->HasFilledPacket()) {
        SendSerialMessage();
    }
    xSemaphoreGiveRecursive(xTXSemaphore);
}

// Compatibility with serial->write
size_t BLESerialHandler::write(uint8_t data)
{
    AppenSerialStack(&data, 1);
    return 1;
}
size_t BLESerialHandler::write(const uint8_t *buffer, size_t size)
{
    AppenSerialStack((uint8_t *)buffer, size);
    return size;
}

void BLESerialHandler::Loop()
{
    if (xSemaphoreGetMutexHolder(xTXSemaphore) == NULL) {
        // Task is not locked, then we can check && send
        xSemaphoreTakeRecursive( xTXSemaphore, portMAX_DELAY );
        SendSerialMessage();
        xSemaphoreGiveRecursive(xTXSemaphore);
    }
}
#endif