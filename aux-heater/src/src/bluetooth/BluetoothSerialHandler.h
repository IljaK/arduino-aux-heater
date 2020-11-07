#pragma once
#include <Arduino.h>
#include "../common/Util.h"
#include "../common/TimeUtil.h"
#include "../serial/SerialCharResponseHandler.h"
#include "../common/DebugHandler.h"

constexpr char BT_CONNECTED_CMD[] = "+CONN";
constexpr char BT_DISCONNECTED_CMD[] = "+DISC";
constexpr char BT_STATS_CMD[] = "+STATS";

class BluetoothSerialHandler : public SerialCharResponseHandler, public DebugHandler
{
private:
	BME1280DataCallback bme280DataCB = NULL;
    BatteryDataCallback batteryDataCB = NULL;
    void SendStats();
public:
	BluetoothSerialHandler(Stream * serial, BME1280DataCallback bme280DataCB, BatteryDataCallback batteryDataCB);
	~BluetoothSerialHandler();

	//void OnTimerComplete(TimerID timerId) override;
	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false) override;
	bool IsBusy() override;

};
