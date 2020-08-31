/*
 Name:    aux-heater.ino
 Created: 11/7/2018 4:04:00 PM
 Author:  Ilja
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "src/common/Timer.h"
#include "src/common/Util.h"
#include "src/AuxHeaterSerial.h"
#include "src/GSMSerialHandler.h"
#include "src/LedController.h"
#include "src/BatteryMonitor.h"

SoftwareSerial auxSerial(AUX_RX_PIN, AUX_TX_PIN);
AuxHeaterSerial auxSerialHandler(&auxSerial);
GSMSerialHandler gsmSerialHandler(&handleSMSCommand, &handleDtmfCommand, &Serial);

SoftwareSerial outSerial(DEBUG_RX_PIN, DEBUG_TX_PIN);

BatteryMonitor batteryMonitor(20000.0f, 4700.0f, &handleLevelChanged);
LedController ledController;

extern Stream *outStream;

void setup() {

	// USB port initialization
	Serial.begin(COMMON_BAUD_RATE);

	outStream = &outSerial;

	auxSerial.begin(AUX_BAUD_RATE);
	ledController.SetFrequency(100, 11, 0b00000001);

	outSerial.begin(DEBUG_BAUD_RATE);

	outPrintf("Setup done!");
}

void loop() {

	updateTime();

	Timer::Loop();

	auxSerialHandler.Loop();
	gsmSerialHandler.Loop();
	ledController.Loop();

}

void handleLevelChanged(VoltageLevelState level) {

	switch (level) {
		case VoltageLevelState::CRITICAL_LEVEL:
		case VoltageLevelState::DEAD_LEVEL:
		case VoltageLevelState::LOW_LEVEL:
		case VoltageLevelState::OVERFLOW_LEVEL:
			gsmSerialHandler.SendSMSMessage(&handleLevelMessage);
			break;
	}
}

void handleSMSCommand(char *command, size_t size, time_t sendTS) {

	time_t now = time(NULL);
	int32_t diff = difftime(now, sendTS);

	char timeString[32];

	tm tmStruct;
	gmtime_r(&now, &tmStruct);
	asctime_r(&tmStruct, timeString);
	outPrintf("Now: %s", timeString);


	gmtime_r(&sendTS, &tmStruct);
	asctime_r(&tmStruct, timeString);
	outPrintf("SMS: %s", timeString);

	outPrintf("handleSMSCommand diff: %ld", diff);

	if (strcasecmp(command, GSM_AUX_ENABLE) == 0) {
		if (diff <= 60) { // 1 min
			auxSerialHandler.LaunchHeater(&handleHeaterComplete);
		}
	}
	else if (strcasecmp(command, GSM_AUX_DISABLE) == 0) {
		if (diff <= 300) { // 5 min
			auxSerialHandler.StopHeater(&handleHeaterComplete);
		}
	}
}

bool handleDtmfCommand(char code) {

	if (code == GSM_AUX_ENABLE_DTFM) {
		auxSerialHandler.LaunchHeater(NULL);
		return true;
	}
	else if (code == GSM_AUX_DISABLE_DTFM) {
		auxSerialHandler.StopHeater(NULL);
		return true;
	}

	return false;
}

bool handleHeaterComplete(Stream *stream) {

	if (stream->available() > 0) {
		char response[32];
		uint8_t bytes[8];
		uint8_t bytesAmount = stream->readBytes(bytes, 8);

		printBytes(response, 32, bytes, bytesAmount);

		outPrintf("AUX: %s", response);
	}
	gsmSerialHandler.NotifyByCallHangUp();
	return true;
}


bool handleLevelMessage(Stream *stream) {
	bool result = false;

	switch (batteryMonitor.CurrentState())
	{
	case VoltageLevelState::CRITICAL_LEVEL:
		stream->write("Critical battery level! ");
		result = true;
		break;
	case VoltageLevelState::DEAD_LEVEL:
		stream->write("Battery died! ");
		result = true;
		break;
	case VoltageLevelState::LOW_LEVEL:
		stream->write("Low battery level! ");
		result = true;
		break;
	case VoltageLevelState::OVERFLOW_LEVEL:
		stream->write("Overflow battery charge! ");
		result = true;
		break;
	default:
		stream->write("Battery level: ");
		result = true;
		break;
	}

	outPrintf("State: %d result: %d", (uint8_t)batteryMonitor.CurrentState(), (uint8_t)result);

	if (result) {
		char resultVoltage[16];
		dtostrf(batteryMonitor.Voltage(), 4, 2, resultVoltage);
		stream->write(resultVoltage);
		stream->write("V");
	}

	return result;
}

/*
void handleUsbCommand(char **arguments, size_t length)
{
	// input string is 32 char array

	//outPrintf("Args %d:", length);
	//for (uint8_t i = 0; i < length; i++) {
	//	outPrintf("%d) %s", i+1, arguments[i]);
	//}
	
	
	if (strncmp(arguments[0], "volts", 5) == 0) {

		char resultVoltage[16];
		dtostrf(batteryMonitor.Voltage(), 15, 2, resultVoltage);

		char analogVoltage[16];
		dtostrf(batteryMonitor.PinVoltage(), 15, 2, analogVoltage);

		outPrintf("A6: %sv (%sv)", resultVoltage, analogVoltage);
	}
	else if (strncmp(arguments[0], "on", 2) == 0) {
		//auxSerialHandler.LaunchHeater(&onHeaterCmdComplete);
	}
	else if (strncmp(arguments[0], "off", 2) == 0) {
		//auxSerialHandler.StopHeater(&onHeaterCmdComplete);
	}
	
	else if (strncmp(arguments[0], GSM_INIT_CMD, strlen(GSM_INIT_CMD)) == 0) {

		//gsmSerialHandler.FlushData();

		char resp[32];
		resp[0] = 0;
		strcpy(resp, arguments[0]);
		strcat(resp, "\r");

		//gsmSerialHandler.WriteData((uint8_t *)resp, (uint8_t)strlen(resp));
	}
}
*/
