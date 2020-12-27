#include "BluetoothSerialHandler.h"

BluetoothSerialHandler::BluetoothSerialHandler(Stream * serial, StringCallback messageCallback):SerialCharResponseHandler(RESPONSE_SEPARATOR, serial)
{
    debugPrint = serial;
    this->messageCallback = messageCallback;
}

BluetoothSerialHandler::~BluetoothSerialHandler()
{
}

void BluetoothSerialHandler::OnResponseReceived(bool isTimeOut, bool isOverFlow)
{
    SerialCharResponseHandler::OnResponseReceived(isTimeOut, isOverFlow);

    Serial.write("BT: ");
    Serial.write(buffer);
    Serial.write("\r\n");

    //size_t size = strlen(buffer);

    if (strncmp(buffer, BT_CONNECTED_CMD, strlen(BT_CONNECTED_CMD)) == 0) {
        isDebugEnabled = true;
    } else if (strncmp(buffer, BT_DISCONNECTED_CMD, strlen(BT_DISCONNECTED_CMD)) == 0) {
        isDebugEnabled = false;
    } else if (messageCallback != NULL) {
        isDebugEnabled = true;
        messageCallback(buffer, strlen(buffer));
    }
}

void BluetoothSerialHandler::SendStats(TemperatureData * temperatureData, BatteryData * batteryData)
{
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage

    serial->write(BT_STATS_CMD);
    serial->write(':');

    serial->print(temperatureData->temperature); // temp1
    serial->write('|');
    serial->print(temperatureData->humidity); // humidity
    serial->write('|');
    serial->print(temperatureData->pressure); // pressure
    serial->write('|');
    serial->print(temperatureData->temperature2); // temp2
    serial->write('|');
    serial->print(batteryData->voltage); // voltage
    serial->write('|');
    serial->print(batteryData->ampers); // ampers
    serial->write('|');
    serial->print(batteryData->calcVoltage); // calculated voltage
    serial->write('|');
    writeASCII(serial, remainRam()); // remain memory
    serial->write('|');
    writeASCII(serial, TimeManager::GetOnlineSeconds()); // online seconds
    outWriteEnd();
}

bool BluetoothSerialHandler::IsBusy()
{
    return SerialCharResponseHandler::IsBusy();
}

// Compatibility with serial->write
size_t BluetoothSerialHandler::write(uint8_t data)
{
    return serial->write(data);
}
size_t BluetoothSerialHandler::write(const uint8_t *buffer, size_t size)
{
    return serial->write(buffer, size);
}

