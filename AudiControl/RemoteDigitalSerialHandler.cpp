#include "RemoteDigitalSerialHandler.h"

RemoteDigitalSerialHandler::RemoteDigitalSerialHandler(SoftwareSerialHandler *serial): SerialTimerResponseHandler(serial)
{
	pinMode(DIGITAL_RX_PIN, INPUT);
	pinMode(DIGITAL_TX_PIN, OUTPUT);
	digitalWrite(DIGITAL_TX_PIN, HIGH);

	// HIGH - Data flow
	// LOW - AT+ commands flow
	pinMode(DIGITAL_SET_PIN, OUTPUT);
	digitalWrite(DIGITAL_SET_PIN, HIGH);
}


RemoteDigitalSerialHandler::~RemoteDigitalSerialHandler()
{
	if (setCommand != NULL) {
		delete(setCommand);
	}
}

void RemoteDigitalSerialHandler::Loop()
{
	SerialTimerResponseHandler::Loop();

	HandleFlowState();

}

void RemoteDigitalSerialHandler::HandleFlowState()
{
	switch (flowState)
	{
	case RemoteFlowState::NONE:
	case RemoteFlowState::WAIT_RESPONSE:
		break;
	case RemoteFlowState::SET_WAIT_LOW:

		// Wait 'set' pin
		flowTimer.Loop();
		if (!flowTimer.IsRunning()) {
			//SerialPrintf("SEND %s", setCommand);
			flowState = RemoteFlowState::WAIT_RESPONSE;
			FlushData();
			WriteData((uint8_t *)setCommand, strlen(setCommand) + 1, (void *)&this->OnATResponse);
		}
		break;
	case RemoteFlowState::SET_WAIT_COMPLETE:
		flowTimer.Loop();
		if (!flowTimer.IsRunning()) {
			flowState = RemoteFlowState::NONE;
		}
		break;
	}
}

void RemoteDigitalSerialHandler::OnResponseReceived(bool IsTimeOut)
{
	SerialTimerResponseHandler::OnResponseReceived(IsTimeOut);
}

void RemoteDigitalSerialHandler::OnATResponse()
{
	if (!IsBusy()) {

		char response[32];
		size_t length = SerialStream()->readBytes(response, 32);
		response[length] = 0;

		if (strncmp("AT+B", setCommand, 4) == 0) {
			SwitchBaudRate(setCommand + 4);
			//PrintSerialConfig();
		}

		isSetSuccess = (strncmp("OK", response, 2) == 0);

		SerialPrintf("HC12: %s", response);
	}

	flowState = RemoteFlowState::SET_WAIT_COMPLETE;
	flowTimer.Start(SERIAL_RESPONSE_TIMEOUT);
	digitalWrite(SERIAL_RESPONSE_TIMEOUT, HIGH);
}


void RemoteDigitalSerialHandler::SwitchBaudRate(char * serialBaudRate)
{
	// TODO: Switch HC12 baud rate
	if (flowState != RemoteFlowState::NONE) {
		//SerialPrintf("RemoteHandler::SwitchBaudRate is busy!");
		return;
	}

	char command[16];
	snprintf(command, 16, "AT+B%s", serialBaudRate);
	SendATCommand(command, 16);
}

void RemoteDigitalSerialHandler::SendATCommand(char * command, uint8_t size)
{
	if (IsBusy()) return;

	FlushData();

	isSetSuccess = false;

	if (setCommand != NULL) {
		delete(setCommand);
	}
	setCommand = new char[size];
	strcpy(setCommand, command);

	digitalWrite(DIGITAL_SET_PIN, LOW);
	flowTimer.Start(SET_REMOTE_DURATION);
	flowState = RemoteFlowState::SET_WAIT_LOW;

}

bool RemoteDigitalSerialHandler::IsSetSucceeded()
{
	return !IsBusy() && isSetSuccess;
}


bool RemoteDigitalSerialHandler::IsBusy()
{
	return SerialTimerResponseHandler::IsBusy() || flowState != RemoteFlowState::NONE;
}

