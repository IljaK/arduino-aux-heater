#pragma once
#include <Arduino.h>
#include "../common/Util.h"
#include "../common/TimeManager.h"
#include "../serial/SerialCharResponseHandler.h"
#include "../common/DebugHandler.h"
#include "src/TemperatureHandler.h"
#include "src/BatteryMonitor.h"

constexpr char BT_CONNECTED_CMD[] = "+CONN";
constexpr char BT_DISCONNECTED_CMD[] = "+DISC";
constexpr char BT_STATS_CMD[] = "+STATS";

class BluetoothSerialHandler : public SerialCharResponseHandler, public DebugHandler
{
private:
    StringCallback messageCallback = NULL;
public:
	BluetoothSerialHandler(Stream * serial, StringCallback messageCallback);
	~BluetoothSerialHandler();

	//void OnTimerComplete(TimerID timerId, uint8_t data) override;
	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false) override;
	bool IsBusy() override;
    
    void SendStats(TemperatureData * temperatureData, BatteryData * batteryData);

    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);

};
