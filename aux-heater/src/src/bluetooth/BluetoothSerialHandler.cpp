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
    writeDouble(serial, bme280Data.temperature, 5, 2); // in temp
    serial->write('|');
    writeDouble(serial, bme280Data.humidity, 5, 2); // humidity
    serial->write('|');
    writeDouble(serial, bme280Data.pressure, 6, 1); // pressure
    serial->write('|');
    writeDouble(serial, batteryData.voltage, 5, 2); // voltage
    serial->write('|');
    writeDouble(serial, batteryData.ampers, 5, 2); // ampers
    serial->write('|');
    writeDouble(serial, batteryData.calcVoltage, 5, 2); // calculated voltage
    serial->write('|');
    writeASCII(serial, remainRam()); // remain memory
    outWriteEnd();
}

bool BluetoothSerialHandler::IsBusy()
{
    return SerialCharResponseHandler::IsBusy();
}

