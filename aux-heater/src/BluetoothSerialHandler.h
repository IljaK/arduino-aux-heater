#pragma once
#include <Arduino.h>
#include "common/Util.h"
#include "common/TimeUtil.h"
#include "serial/SerialCharResponseHandler.h"
#include "serial/DebugSerialHandler.h"
#include <Adafruit_BME280.h>
#include "BatteryMonitor.h"

constexpr char BT_CONNECTED_CMD[] = "+CONN";
constexpr char BT_DISCONNECTED_CMD[] = "+DISC";
constexpr char BT_STATS_CMD[] = "+STATS";

extern Adafruit_BME280 bme280;
extern BatteryMonitor batteryMonitor;

class BluetoothSerialHandler : public SerialCharResponseHandler, public DebugSerialHandler
{
private:
    void SendStats();
public:
	BluetoothSerialHandler(Stream * serial);
	~BluetoothSerialHandler();

	//void OnTimerComplete(TimerID timerId) override;
	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false) override;
	bool IsBusy() override;

};
