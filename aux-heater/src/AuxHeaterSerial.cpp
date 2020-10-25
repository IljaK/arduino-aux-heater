#include "AuxHeaterSerial.h"

AuxHeaterSerial::AuxHeaterSerial(Stream * auxSerial):SerialTimerResponseHandler(auxSerial)
{
	
}

AuxHeaterSerial::~AuxHeaterSerial()
{
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
	serial->flush();
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

	digitalWrite(AUX_TX_PIN, LOW);
	//delayMicroseconds(300000u);
	delay(300u);
	digitalWrite(AUX_TX_PIN, HIGH);
	delayMicroseconds(59432u);
	serial->write((uint8_t)145u);
	delayMicroseconds(2400u);
	serial->write((uint8_t)129u);

	StopTimer();
	messageTimer = Timer::Start(this, 162124ul);
}
void AuxHeaterSerial::StopCMD()
{
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
	serial->write((uint8_t)170u);
	delayMicroseconds(2400u);
	serial->write((uint8_t)139u);

	StopTimer();
	messageTimer = Timer::Start(this, 152016ul);
}

void AuxHeaterSerial::HandleResult()
{
	cmdAttempt = 0;
	cmdState = HeaterCmdState::NONE;

	if (awaitState != HeaterCmdState::NONE) {
		serial->flush();
		HeaterCmdState state = awaitState;
		awaitState = HeaterCmdState::NONE;
		HandleCMD(state, actionCallback);
		return;
	}

	StreamCallback cb = actionCallback;
	actionCallback = NULL;

	if (cb != NULL) cb(serial);
}

void AuxHeaterSerial::OnTimerComplete(TimerID timerId)
{
	if (messageTimer == timerId) {
		messageTimer = 0;

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
			if (serial->available()) HandleResult();
			break;
		}
	} else {
		SerialTimerResponseHandler::OnTimerComplete(timerId);
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
	return SerialTimerResponseHandler::IsBusy();
}

unsigned long AuxHeaterSerial::ResponseByteTimeOut()
{
	return 3000u;
}

void AuxHeaterSerial::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	SerialTimerResponseHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}