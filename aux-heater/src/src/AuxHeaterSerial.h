#pragma once
#include "Definitions.h"
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

class AuxHeaterSerial: public ITimerCallback
{
private:
    HardwareSerial *pHSerail = NULL;
    Stream *pStream = NULL;
    TimerID flowTimer;

	uint8_t cmdAttempt = 0;
	StreamCallback actionCallback = NULL;

	HeaterCmdState cmdState = HeaterCmdState::NONE;
	HeaterCmdState awaitState = HeaterCmdState::NONE;

	void HandleCMD(HeaterCmdState cmd, StreamCallback actionCallback);
	void LaunchCMD();
	void StopCMD();
	void HandleResult();
    void StopTimer();

public:
	AuxHeaterSerial(HardwareSerial * auxSerial);
	AuxHeaterSerial(Stream * auxSerial);
	virtual ~AuxHeaterSerial();

	void LaunchHeater(StreamCallback resultCallback);
	void StopHeater(StreamCallback resultCallback);

	bool IsBusy();

	void OnTimerComplete(TimerID timerId, uint8_t data) override;
};

