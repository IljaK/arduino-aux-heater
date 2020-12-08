#include "BluetoothSerialHandler.h"

BluetoothSerialHandler::BluetoothSerialHandler(Stream * serial, BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB):SerialCharResponseHandler(RESPONSE_SEPARATOR, serial)
{
    debugPrint = serial;
    this->bme280DataCB = bme280DataCB;
    this->batteryDataCB = batteryDataCB;
}

BluetoothSerialHandler::~BluetoothSerialHandler()
{
}

void BluetoothSerialHandler::OnResponseReceived(bool isTimeOut, bool isOverFlow)
{
    SerialCharResponseHandler::OnResponseReceived(isTimeOut, isOverFlow);

    //size_t size = strlen(buffer);

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

    BME280Data bme280Data;
    if (bme280DataCB != NULL) bme280DataCB(&bme280Data);

    BatteryData batteryData;
    if (batteryDataCB != NULL) batteryDataCB(&batteryData);

    serial->write(BT_STATS_CMD);
    serial->write(':');


    // TODO: read voltage
    //outWrite(temperature, 5, 2); // out temp
    serial->write("0.0");
    serial->write('|');
    serial->write(bme280Data.temperature); // in temp
    serial->write('|');
    serial->write(bme280Data.humidity); // humidity
    serial->write('|');
    serial->write(bme280Data.pressure); // pressure
    serial->write('|');
    serial->write(batteryData.voltage); // voltage
    serial->write('|');
    serial->write(batteryData.ampers); // ampers
    serial->write('|');
    serial->write(batteryData.calcVoltage); // calculated voltage
    serial->write('|');
    writeASCII(serial, remainRam()); // remain memory
    outWriteEnd();
}

bool BluetoothSerialHandler::IsBusy()
{
    return SerialCharResponseHandler::IsBusy();
}

