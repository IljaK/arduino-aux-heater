
/*
 Name:    aux-heater.ino
 Created: 11/7/2018 4:04:00 PM
 Author:  Ilja
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <Adafruit_BME280.h>
//#include <DallasTemperature.h>
//#include "libs/ELECHOUSE_CC1101.h"
#include "src/common/Timer.h"
#include "src/common/Util.h"
#include "src/AuxHeaterSerial.h"
#include "src/GSMSerialHandler.h"
#include "src/LedController.h"
#include "src/BatteryMonitor.h"
#include "src/serial/DebugSerialHandler.h"
#include "src/BluetoothSerialHandler.h"

SoftwareSerial auxSerial(AUX_RX_PIN, AUX_TX_PIN);
AuxHeaterSerial auxSerialHandler(&auxSerial);
GSMSerialHandler gsmSerialHandler(&handleSMSCommand, &handleDtmfCommand, NULL/*&Serial*/);

//SoftwareSerial outSerial(DEBUG_RX_PIN, DEBUG_TX_PIN);
AltSoftSerial outSerial;
BluetoothSerialHandler btSerialHandler(&outSerial, &handleStatsRequest);

BatteryMonitor batteryMonitor(4700.0f, 2200.0f, &handleLevelChanged);
//LedController ledController;

Adafruit_BME280 bme280;
//DallasTemperature tempSensor;

constexpr uint8_t CC1101_BUFFER_SIZE = 12;
byte RX_buffer[CC1101_BUFFER_SIZE];

void setup() {

	// USB port initialization
	Serial.begin(SERIAL_BAUD_RATE);

	auxSerial.begin(AUX_BAUD_RATE);
    auxSerial.stopListening();

	//ledController.SetFrequency(100, 11, 0b00000001);

	outSerial.begin(SERIAL_BAUD_RATE);
	outSerial.listen();

	bme280.begin(246u);

	//pinMode(SCK_PIN, OUTPUT);
  	//pinMode(MOSI_PIN, OUTPUT);
  	//pinMode(SS_PIN, OUTPUT);
  	//pinMode(MISO_PIN, INPUT);
	//pinMode(GDO0, INPUT);
	//digitalWrite(SS_PIN, HIGH);
	//digitalWrite(SCK_PIN, HIGH);
	//digitalWrite(MOSI_PIN, LOW);

	//ELECHOUSE_cc1101.Init(F_433);
	//ELECHOUSE_cc1101.SetReceive();

	DebugSerialHandler::outWrite(F("Setup done!\r\n"), true);
}

void loop() {

	updateTime();

	Timer::Loop();

	auxSerialHandler.Loop();
	gsmSerialHandler.Loop();
	//ledController.Loop();
	btSerialHandler.Loop();

	//if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
	//	uint8_t size = ELECHOUSE_cc1101.ReceiveData(RX_buffer, CC1101_BUFFER_SIZE);
//
	//	DebugSerialHandler::outWrite(F("CC1101: "));
	//	DebugSerialHandler::PrintBytes(RX_buffer, size);
	//	DebugSerialHandler::outWriteEnd();
	//}

}

bool handleStatsRequest(Stream *stream) {
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage

    float temperature = bme280.readTemperature();
    float humidity = bme280.readHumidity();
    float pressure = bme280.readPressure() / 100.0f;

    // TODO: read voltage
    //outWrite(temperature, 5, 2); // out temp
    stream->write("0.0");
    stream->write('|');
    writeDouble(stream, temperature, 5, 2); // in temp
    stream->write('|');
    writeDouble(stream, humidity, 5, 2); // humidity
    stream->write('|');
    writeDouble(stream, pressure, 6, 1); // pressure
    stream->write('|');
    stream->write("0.0"); // voltage
    stream->write('|');
    stream->write("0.0"); // ampers
    stream->write('|');
    stream->write("0.0"); // calculated voltage
    stream->write('|');
    writeASCII(stream, remainRam()); // remain memory

	return true;
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

void handleSMSCommand(char *command, size_t size, time_t smsDispatchUTCts) {

	time_t now = time(NULL);
	
	int32_t diff = difftime(now, smsDispatchUTCts);

	if (strcasecmp(command, "on") == 0) {
		if (diff <= 60) { // 1 min
			auxSerialHandler.LaunchHeater(&handleHeaterComplete);
		}
	}
	else if (strcasecmp(command, "off") == 0) {
		if (diff <= 300) { // 5 min
			auxSerialHandler.StopHeater(&handleHeaterComplete);
		}
	}
}

bool handleDtmfCommand(char code) {

	if (code == '1') {
		auxSerialHandler.LaunchHeater(NULL);
		return true;
	}
	else if (code == '0') {
		auxSerialHandler.StopHeater(NULL);
		return true;
	}

	return false;
}

bool handleHeaterComplete(Stream *stream) {

	if (stream->available() > 0) {
		uint8_t bytes[8];
		uint8_t bytesAmount = stream->readBytes(bytes, 8);

        if (DebugSerialHandler::IsDebugEnabled()) {

            DebugSerialHandler::outWrite(F("AUX: "));
			for (uint8_t i = 0; i < bytesAmount; i++) {
				DebugSerialHandler::outWriteASCII(bytes[i]);
			}
			DebugSerialHandler::outWriteEnd();
        }
	}
	gsmSerialHandler.NotifyByCallHangUp();
	return true;
}


bool handleLevelMessage(Stream *stream) {
	bool result = false;

	switch (batteryMonitor.CurrentState())
	{
	case VoltageLevelState::CRITICAL_LEVEL:
		stream->print(F("Critical battery level! "));
		result = true;
		break;
	case VoltageLevelState::DEAD_LEVEL:
		stream->print(F("Battery died! "));
		result = true;
		break;
	case VoltageLevelState::LOW_LEVEL:
		stream->print(F("Low battery level! "));
		result = true;
		break;
	case VoltageLevelState::OVERFLOW_LEVEL:
		stream->print(F("Overflow battery charge! "));
		result = true;
		break;
	default:
		stream->print(F("Battery level: "));
		result = true;
		break;
	}

    if (DebugSerialHandler::IsDebugEnabled()) {
        DebugSerialHandler::outWrite(F("State: "));
        DebugSerialHandler::outWriteASCII((uint8_t)batteryMonitor.CurrentState());
        DebugSerialHandler::outWrite(F(" result: "));
        DebugSerialHandler::outWriteASCII((uint8_t)result);
		DebugSerialHandler::outWriteEnd();
    }

	if (result) {
		writeDouble(stream, batteryMonitor.Voltage(), 4, 2);
		stream->write("V");
	}

	return result;
}
