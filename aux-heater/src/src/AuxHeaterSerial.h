#pragma once
#include "common/Util.h"
#include "common/Timer.h"
#include "serial/BaseSerialHandler.h"
#include "serial/SerialTimerResponseHandler.h"

enum class HeaterCmdState : uint8_t
{
	NONE,
	LAUNCH,
	STOP
};

const uint8_t AUX_MAX_ATTEMPTS = 3;

class AuxHeaterSerial: public SerialTimerResponseHandler
{
private:
	uint8_t cmdAttempt = 0;
	StreamCallback actionCallback = NULL;

	HeaterCmdState cmdState = HeaterCmdState::NONE;
	HeaterCmdState awaitState = HeaterCmdState::NONE;

	void HandleCMD(HeaterCmdState cmd, StreamCallback actionCallback);
	void LaunchCMD();
	void StopCMD();
	void HandleResult();

public:
	AuxHeaterSerial(Stream * auxSerial);
	~AuxHeaterSerial();

	void LaunchHeater(StreamCallback resultCallback);
	void StopHeater(StreamCallback resultCallback);

	bool IsBusy() override;
	unsigned long ResponseByteTimeOut() override;

	void OnResponseReceived(bool IsTimeOut, bool isOverFlow) override;
	void OnTimerComplete(TimerID timerId, uint8_t data) override;
};

