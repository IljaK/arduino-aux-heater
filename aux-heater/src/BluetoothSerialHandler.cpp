#include "BluetoothSerialHandler.h"

BluetoothSerialHandler::BluetoothSerialHandler(Stream * serial):SerialCharResponseHandler(RESPONSE_SEPARATOR, serial)
{
    debugStream = serial;
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
    } else if (strncmp(buffer, BT_CONNECTED_CMD, strlen(BT_CONNECTED_CMD) == 0)) {
        isDebugEnabled = true;
    } else if (strncmp(buffer, BT_DISCONNECTED_CMD, strlen(BT_DISCONNECTED_CMD) == 0)) {
        isDebugEnabled = false;
    } 
}

void BluetoothSerialHandler::SendStats()
{
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage

    float temperature = bme280.readTemperature();
    float humidity = bme280.readHumidity();
    float pressure = bme280.readPressure() / 100.0f;

    // TODO: read voltage

    serial->write(BT_STATS_CMD);
    serial->write(':');
    outWrite(temperature, 5, 2); // out temp
    serial->write('|');
    outWrite(temperature, 5, 2); // in temp
    serial->write('|');
    outWrite(humidity, 5, 2); // humidity
    serial->write('|');
    outWrite(pressure, 6, 1); // pressure
    serial->write('|');
    serial->write("0.0"); // voltage
    serial->write('|');
    serial->write("0.0"); // ampers
    serial->write('|');
    serial->write("0.0"); // calculated voltage
    serial->write('|');
    outWriteASCII(remainRam()); // remain memory
    outWriteEnd();

    outWriteEnd();
}

bool BluetoothSerialHandler::IsBusy()
{
    return SerialCharResponseHandler::IsBusy();
}

