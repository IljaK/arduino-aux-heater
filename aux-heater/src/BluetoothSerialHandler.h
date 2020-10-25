#pragma once
#include <Arduino.h>
#include "common/Util.h"
#include "common/TimeUtil.h"
#include "serial/SerialCharResponseHandler.h"
#include "serial/DebugSerialHandler.h"

constexpr char BT_CONNECTED_CMD[] = "+CONN";
constexpr char BT_DISCONNECTED_CMD[] = "+DISC";
constexpr char BT_STATS_CMD[] = "+STATS";

class BluetoothSerialHandler : public SerialCharResponseHandler, public DebugSerialHandler
{
private:
	StreamCallback statsCallback = NULL;
    void SendStats();
public:
	BluetoothSerialHandler(Stream * serial, StreamCallback statsCallback);
	~BluetoothSerialHandler();

	//void OnTimerComplete(TimerID timerId) override;
	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false) override;
	bool IsBusy() override;

};
