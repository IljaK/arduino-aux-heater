#include "BluetoothSerialHandler.h"

BluetoothSerialHandler::BluetoothSerialHandler(Stream * serial, StreamCallback statsCallback):SerialCharResponseHandler(RESPONSE_SEPARATOR, serial)
{
    debugStream = serial;
    this->statsCallback = statsCallback;
}

BluetoothSerialHandler::~BluetoothSerialHandler()
{
}

void BluetoothSerialHandler::OnResponseReceived(bool isTimeOut, bool isOverFlow)
{
    SerialCharResponseHandler::OnResponseReceived(isTimeOut, isOverFlow);

    size_t size = strlen(buffer);

    if (strcmp(buffer, BT_STATS_CMD) == 0) {
        isDebugEnabled = true;
        SendStats();
    } else if (strncmp(buffer, BT_CONNECTED_CMD, strlen(BT_CONNECTED_CMD)) == 0) {
        isDebugEnabled = true;
    } else if (strncmp(buffer, BT_DISCONNECTED_CMD, strlen(BT_DISCONNECTED_CMD)) == 0) {
        isDebugEnabled = false;
    } 
}

void BluetoothSerialHandler::SendStats()
{
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage

    serial->write(BT_STATS_CMD);
    serial->write(':');
    if (statsCallback != NULL) {
        statsCallback(serial);
    }
    outWriteEnd();
}

bool BluetoothSerialHandler::IsBusy()
{
    return SerialCharResponseHandler::IsBusy();
}

