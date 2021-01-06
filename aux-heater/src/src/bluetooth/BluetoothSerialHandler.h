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

constexpr char BT_TEMP_CMD[] = "+TEMP";
constexpr char BT_BAT_CMD[] = "+BAT";
constexpr char BT_DEV_CMD[] = "+DEV";

enum StatPartType:uint8_t
{
    TEMP,
    BAT,
    DEV
};

class BluetoothSerialHandler : public SerialCharResponseHandler, public DebugHandler
{
private:
    StringCallback messageCallback = NULL;
    TimerID statsTimer = 0;
    void StopStatsTimer();
    void SendStat(StatPartType stat);

public:
	BluetoothSerialHandler(Stream * serial, StringCallback messageCallback);
	~BluetoothSerialHandler();

	//void OnTimerComplete(TimerID timerId, uint8_t data) override;
	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false) override;
    void OnTimerComplete(TimerID timerId, uint8_t data) override;
	bool IsBusy() override;
    
    void SendTemp();
    void SendBat();
    void SendDev();

    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);

};
