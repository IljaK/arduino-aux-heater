#include "AuxHeaterSerial.h"
#include <typeinfo>

AuxHeaterSerial::AuxHeaterSerial(HardwareSerial * auxSerial):ITimerCallback()
{
    this->pHSerail = auxSerial;
    this->pStream = auxSerial;
}

AuxHeaterSerial::AuxHeaterSerial(Stream * auxSerial):ITimerCallback()
{
    this->pHSerail = NULL;
    this->pStream = auxSerial;
}

AuxHeaterSerial::~AuxHeaterSerial()
{
}

void AuxHeaterSerial::StopTimer()
{
    if (flowTimer != 0) {
        Timer::Stop(flowTimer);
        flowTimer = 0;
    }
}

void AuxHeaterSerial::LaunchHeater(StreamCallback resultCallback)
{
	HandleCMD(HeaterCmdState::LAUNCH, resultCallback);
}

void AuxHeaterSerial::HandleCMD(HeaterCmdState cmd, StreamCallback actionCallback) {
	if (IsBusy()) {
		if (cmdState != cmd) {
			awaitState = cmd;
			this->actionCallback = actionCallback;
		}
		return;
	}
	pStream->flush();
	cmdState = cmd;
	this->actionCallback = actionCallback;
	cmdAttempt = 0;

	switch (cmd)
	{
	case HeaterCmdState::LAUNCH:
		LaunchCMD();
		break;
	case HeaterCmdState::STOP:
		StopCMD();
		break;
	default:
		break;
	}
}

void AuxHeaterSerial::StopHeater(StreamCallback actionCallback)
{
	HandleCMD(HeaterCmdState::STOP, actionCallback);
}

void AuxHeaterSerial::LaunchCMD()
{
	/*
//for (uint8_t i = 0; i < 3; i++) {
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(287100);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(59432);

	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(424);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(400);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(1236);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(396);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(860);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(3592);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(408);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(404);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(2476);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(162124);
//}
*/
	cmdAttempt++;

    pStream->flush();
    if (pHSerail != NULL) {
        pHSerail->end();
    }
    pinMode(AUX_TX_PIN, OUTPUT);
	digitalWrite(AUX_TX_PIN, LOW);

	delay(300u);

    digitalWrite(AUX_TX_PIN, HIGH);
    if (pHSerail != NULL) {
        #if defined(MKRGSM1400)
        pinPeripheral(AUX_TX_PIN, PIO_SERCOM);
        #endif
        pHSerail->begin(AUX_BAUD_RATE, SERIAL_8N1);
    }
	delayMicroseconds(59432u);

	pStream->write((uint8_t)145u);
	//delayMicroseconds(2400u);
	pStream->write((uint8_t)129u);

	StopTimer();
	flowTimer = Timer::Start(this, 162124ul);
}
void AuxHeaterSerial::StopCMD()
{

    // 0 01010101 - (3200 delay) - 0 11010001
	// [0](824)-[1](424)-[0](424)-[1](392)-[0](428)-[1](396)-[0](392)-[1](3596)-[0](436)-[1](800)-[0](428)-[1](392)-[0](1252)-[1](152016)
	/*
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(824);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(424);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(424);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(392);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(428);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(396);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(392);

	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(3596);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(436);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(800);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(428);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(392);
	digitalWrite(AUX_TX_PIN, LOW);
	delayMicroseconds(1252);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(152016);
	*/

	cmdAttempt++;
	pStream->write((uint8_t)170u);
    //delayMicroseconds(2400u);
	pStream->write((uint8_t)139u);

	StopTimer();
	flowTimer = Timer::Start(this, 152016ul);
}

void AuxHeaterSerial::HandleResult()
{
	cmdAttempt = 0;
	cmdState = HeaterCmdState::NONE;

	if (awaitState != HeaterCmdState::NONE) {
		pStream->flush();
		HeaterCmdState state = awaitState;
		awaitState = HeaterCmdState::NONE;
		HandleCMD(state, actionCallback);
		return;
	}

	StreamCallback cb = actionCallback;
	actionCallback = NULL;

	if (cb != NULL) cb(pStream);
}

void AuxHeaterSerial::OnTimerComplete(TimerID timerId, uint8_t data)
{
	if (flowTimer == timerId) {
		flowTimer = 0;

		switch (cmdState)
		{
		case HeaterCmdState::LAUNCH:
			if (cmdAttempt >= AUX_MAX_ATTEMPTS) HandleResult();
			else LaunchCMD();
			break;
		case HeaterCmdState::STOP:
			if (cmdAttempt >= AUX_MAX_ATTEMPTS) HandleResult();
			else StopCMD();
			break;
		default:
			if (pStream->available()) HandleResult();
			break;
		}
	} else {
		ITimerCallback::OnTimerComplete(timerId, data);
	}
}

bool AuxHeaterSerial::IsBusy() {
	switch (cmdState)
	{
		case HeaterCmdState::LAUNCH:
		case HeaterCmdState::STOP:
			return true;
		default:
			break;
	}
	return false;
}
