
/*
 Name:    aux-heater.ino
 Created: 11/7/2018 4:04:00 PM
 Author:  Ilja
*/

#include <Arduino.h>
//#include <SoftwareSerial.h>
#include <Adafruit_BME280.h>
//#include <DallasTemperature.h>
//#include "libs/ELECHOUSE_CC1101.h"
#include "src/common/Timer.h"
#include "src/common/Util.h"
#include "src/AuxHeaterSerial.h"
#include "src/GSMSerialHandler.h"
#include "src/BatteryMonitor.h"
#include "src/common/DebugHandler.h"
#include "src/bluetooth/BluetoothSerialHandler.h"
#include "src/bluetooth/BLEHandler.h"

AuxHeaterSerial auxSerialHandler(&Serial2);
GSMSerialHandler gsmSerialHandler(&handleSMSCommand, &handleDtmfCommand, NULL/*&Serial*/);

//BluetoothSerialHandler btSerialHandler(&Serial, &getBME280Data, &getBatteryData);

BLEHandler bleHandler(getBME280Data, getBatteryData);

BatteryMonitor batteryMonitor(4700.0f, 2200.0f, &handleLevelChanged);
//LedController ledController;

Adafruit_BME280 bme280;
//DallasTemperature tempSensor;

constexpr uint8_t CC1101_BUFFER_SIZE = 12;
byte RX_buffer[CC1101_BUFFER_SIZE];

void setup() {

    // USB Serial
    Serial.begin(SERIAL_BAUD_RATE);
    while(!Serial) {}
    // AUX heater serial
    Serial2.begin(AUX_BAUD_RATE);

    //ledController.SetFrequency(100, 11, 0b00000001);

    //outSerial.begin(SERIAL_BAUD_RATE);
    //outSerial.listen();

    bme280.begin(246u);
    //bool state = bme280.begin(246u, &Wire);
    //Serial.print("bme280: ");
    //Serial.print(state);
    //Serial.println();

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

    bleHandler.Start();

    DebugHandler::outWrite(F("Setup done!\r\n"), true);
}

void loop() {

    updateTime();

    Timer::Loop();

    bleHandler.Loop();
    auxSerialHandler.Loop();
    gsmSerialHandler.Loop();
    //ledController.Loop();
    //btSerialHandler.Loop();

    //if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
    //	uint8_t size = ELECHOUSE_cc1101.ReceiveData(RX_buffer, CC1101_BUFFER_SIZE);
    //
    //	DebugHandler::outWrite(F("CC1101: "));
    //	DebugHandler::PrintBytes(RX_buffer, size);
    //	DebugHandler::outWriteEnd();
    //}

}

void getBME280Data(BME280Data* data) {
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage
    if (data == NULL) return;

    data->temperature = bme280.readTemperature();
    data->humidity = bme280.readHumidity();
    data->pressure = bme280.readPressure();

    // TODO:

}

void getBatteryData(BatteryData* data) {
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage
    if (data == NULL) return;

    data->voltage = batteryMonitor.Voltage();
    data->ampers = 0;
    data->calcVoltage = 0;

    // TODO:

}

void handleLevelChanged(VoltageLevelState level) {

    switch (level) {
    case VoltageLevelState::CRITICAL_LEVEL:
    case VoltageLevelState::DEAD_LEVEL:
    case VoltageLevelState::LOW_LEVEL:
    case VoltageLevelState::OVERFLOW_LEVEL:
        gsmSerialHandler.SendSMSMessage(&handleLevelMessage);
        break;
    case VoltageLevelState::NORMAL_LEVEL:
    case VoltageLevelState::CHARGE_LEVEL:
        // DO Nothing, OK
        break;
    }
}

void handleSMSCommand(char* command, size_t size, time_t smsDispatchUTCts) {

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

bool handleHeaterComplete(Stream* stream) {

    if (stream->available() > 0) {
        uint8_t bytes[8];
        uint8_t bytesAmount = stream->readBytes(bytes, 8);

        if (DebugHandler::IsDebugEnabled()) {

            DebugHandler::outWrite(F("AUX: "));
            for (uint8_t i = 0; i < bytesAmount; i++) {
                DebugHandler::outWriteASCII(bytes[i]);
            }
            DebugHandler::outWriteEnd();
        }
    }
    gsmSerialHandler.NotifyByCallHangUp();
    return true;
}


bool handleLevelMessage(Stream* stream) {
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

    if (DebugHandler::IsDebugEnabled()) {
        DebugHandler::outWrite(F("State: "));
        DebugHandler::outWriteASCII((uint8_t)batteryMonitor.CurrentState());
        DebugHandler::outWrite(F(" result: "));
        DebugHandler::outWriteASCII((uint8_t)result);
        DebugHandler::outWriteEnd();
    }

    if (result) {
        writeDouble(stream, batteryMonitor.Voltage(), 4, 2);
        stream->write("V");
    }

    return result;
}