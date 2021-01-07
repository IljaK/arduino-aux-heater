#include "BluetoothSerialHandler.h"

extern TemperatureHandler temperatureHandler;
extern BatteryMonitor batteryMonitor;
extern uint8_t btPinState;

BluetoothSerialHandler::BluetoothSerialHandler(Stream * serial, StringCallback messageCallback):SerialCharResponseHandler(RESPONSE_SEPARATOR, serial)
{
    isDebugEnabled = btPinState;
    debugPrint = serial;
    this->messageCallback = messageCallback;
}

BluetoothSerialHandler::~BluetoothSerialHandler()
{
}

void BluetoothSerialHandler::OnTimerComplete(TimerID timerId, uint8_t data)
{
	if (statsTimer == timerId) {
		statsTimer = 0;
        SendStat((StatPartType)data);
	} else {
        SerialCharResponseHandler::OnTimerComplete(timerId, data);
    }
}

void BluetoothSerialHandler::StopStatsTimer()
{
    if (statsTimer != 0) {
        Timer::Stop(statsTimer);
        statsTimer = 0;
    }
}

void BluetoothSerialHandler::SendStat(StatPartType stat)
{
    StopStatsTimer();
    if (!isDebugEnabled) {
        return;
    }
    switch (stat)
    {
    case StatPartType::TEMP:
        SendTemp();
        statsTimer = Timer::Start(this, 100000ul, StatPartType::BAT);
        break;
    case StatPartType::BAT:
        SendBat();
        statsTimer = Timer::Start(this, 100000ul, StatPartType::DEV);
        break;
    case StatPartType::DEV:
        SendDev();
        break;
    default:
        break;
    }
}

void BluetoothSerialHandler::Loop()
{    if (isDebugEnabled != btPinState) {
        if (!isDebugEnabled) {
            // Connected
        } else {
            // Disconnected
            StopStatsTimer();
        }
    }
    isDebugEnabled = btPinState;
    SerialCharResponseHandler::Loop();
}

void BluetoothSerialHandler::OnResponseReceived(bool isTimeOut, bool isOverFlow)
{
    SerialCharResponseHandler::OnResponseReceived(isTimeOut, isOverFlow);

    //size_t size = strlen(buffer);

    if (strncmp(buffer, BT_STATS_CMD, strlen(BT_STATS_CMD)) == 0) {
        if (statsTimer == 0) {
            SendStat(StatPartType::TEMP);
        }
    } else if (messageCallback != NULL) {
        messageCallback(buffer, strlen(buffer));
    }
}

void BluetoothSerialHandler::SendTemp()
{
    if (!isDebugEnabled) {
        return;
    }

    TemperatureData temperatureData;
    temperatureHandler.GetTemperature(&temperatureData);

    //serial->clearWriteError();
    //serial->flush();
    serial->write(BT_TEMP_CMD);
    //Serial.write(BT_TEMP_CMD);
    serial->write(':');
    //Serial.write(':');
    serial->print(temperatureData.temperature); // temp1
    //Serial.print(temperatureData->temperature);
    serial->write('|');
    //Serial.write('|');
    serial->print(temperatureData.humidity); // humidity
    //Serial.print(temperatureData->humidity);
    serial->write('|');
    //Serial.write('|');
    serial->print(temperatureData.pressure); // pressure
    //Serial.print(temperatureData->pressure);
    serial->write('|');
    //Serial.write('|');
    serial->print(temperatureData.temperature2); // temp2
    //Serial.print(temperatureData->temperature2);
    serial->print(RESPONSE_SEPARATOR);
    //Serial.write("\r\n");
}
void BluetoothSerialHandler::SendBat()
{
    if (!isDebugEnabled) {
        return;
    }
    BatteryData batteryData;
    batteryMonitor.GetBatteryData(&batteryData);

    serial->write(BT_BAT_CMD);
    serial->write(':');
    serial->print(batteryData.voltage); // voltage
    serial->write('|');
    serial->print(batteryData.pinVoltage); // pinVoltage
    serial->write('|');
    serial->print(batteryData.ampers, 3); // ampers
    serial->write('|');
    serial->print(batteryData.calcVoltage); // calculated voltage
    serial->print(RESPONSE_SEPARATOR);
}
void BluetoothSerialHandler::SendDev()
{
    if (!isDebugEnabled) {
        return;
    }
    serial->write(BT_DEV_CMD);
    serial->write(':');
    writeASCII(serial, remainRam()); // remain memory
    serial->write('|');
    writeASCII(serial, TimeManager::GetOnlineSeconds()); // online seconds
    serial->print(RESPONSE_SEPARATOR);
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

