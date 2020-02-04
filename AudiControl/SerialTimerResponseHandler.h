#pragma once
#include "Util.h"
#include "Timer.h"
#include "BaseSerialHandler.h"

class SerialTimerResponseHandler: public BaseSerialHandler
{
protected:
	Timer messageTimer;
	uint8_t registeredBytes = 0;
	inline bool IsLimitReached();

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

	virtual unsigned long ResponseByteTimeOut();

	double SingleByteTransferDuration();
};

