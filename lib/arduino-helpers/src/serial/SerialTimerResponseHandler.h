#pragma once
#include "../common/Util.h"
#include "../common/Timer.h"
#include "BaseSerialHandler.h"

class SerialTimerResponseHandler: public BaseSerialHandler
{
protected:
	TimerID messageTimer = 0;
	uint8_t registeredBytes = 0;

	void StartTimer();
	void StopTimer();

public:
	SerialTimerResponseHandler(Stream * serial);
	~SerialTimerResponseHandler();

	void Loop() override;
	bool IsBusy() override;

	virtual uint8_t GetDataBitsAmount();
	virtual uint8_t GetStopBitsAmount();
	virtual uint32_t GetBaudRate();

	void FlushData() override;
	void ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow = false) override;
	void OnTimerComplete(TimerID timerId, uint8_t data) override;
    void OnTimerStop(TimerID timerId, uint8_t data) override;

	virtual unsigned long ResponseByteTimeOut();

	double SingleByteTransferDuration();
};

