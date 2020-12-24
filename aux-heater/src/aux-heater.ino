
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
#include "src/BatteryMonitor.h"
#include "src/common/DebugHandler.h"
#include <strings.h>
#include "src/bluetooth/BLEHandler.h"
#include "src/bluetooth/BluetoothSerialHandler.h"
#include "src/common/BinaryMessageStack.h"

void handleLevelChanged(VoltageLevelState level);
void handleSMSCommand(char* command, size_t size, time_t smsDispatchUTCts);
bool handleDtmfCommand(char code);
bool handleLevelMessage(Stream* stream);

#if ESP32
    #include "src/gsm/SimomGSMHandler.h"
    void handleBLEMessage(BinaryMessage * message);

    HardwareSerial auxSerial(1);
    HardwareSerial gsmSerial(2);
    AuxHeaterSerial auxSerialHandler(&auxSerial);
    GSMSerialHandler gsmSerialHandler(&handleSMSCommand, &handleDtmfCommand, &gsmSerial);
    BLEHandler bleHandler(handleBLEMessage, getBME280Data, getBatteryData);
#elif MKRGSM1400
    #include "src/gsm/UbloxGSMHandler.h"
    AuxHeaterSerial auxSerialHandler(&Serial1);
    UbloxGSMHandler gsmSerialHandler(handleSMSCommand, handleDtmfCommand, &SerialGSM);
#else
    #include "src/gsm/SimcomGSMHandler.h"
    //BluetoothSerialHandler btSerialHandler(&Serial, &getBME280Data, &getBatteryData);
    AuxHeaterSerial auxSerialHandler(&Serial1);
    SimcomGSMHandler gsmSerialHandler(handleSMSCommand, handleDtmfCommand, &Serial2);
#endif


BatteryMonitor batteryMonitor(4700.0f, 2200.0f, handleLevelChanged);
//LedController ledController;

Adafruit_BME280 bme280;
//DallasTemperature tempSensor;

void setup() {
    // USB Serial
    Serial.begin(SERIAL_BAUD_RATE);
    while(!Serial) {}

#if ESP32
    // AUX heater serial
    auxSerial.begin(AUX_BAUD_RATE, SERIAL_8N1, AUX_RX_PIN, AUX_TX_PIN);
    // gsm serial
    gsmSerial.begin(SERIAL_BAUD_RATE, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);

    bleHandler.Start();
#elif MKRGSM1400
    pinMode(GSM_RESETN, OUTPUT);
    digitalWrite(GSM_RESETN, LOW);
    pinMode(GSM_RTS, OUTPUT);
    pinMode(GSM_CTS, INPUT);
    digitalWrite(GSM_RTS, LOW);

    DebugHandler::SetPrint(&Serial);
    SerialGSM.begin(SERIAL_BAUD_RATE, SERIAL_8N1);
#endif

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

    gsmSerialHandler.Start();
    DebugHandler::outWrite(F("Setup done!\r\n"));
}

void loop() {
    Timer::Loop();

#if ESP32
    bleHandler.Loop();
#endif

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

    if (Serial.available() > 0) {
        while (Serial.available() > 0) {
            SerialGSM.write(Serial.read());
        }
    }

    // Must be latest task loop
    TimeManager::LateLoop();
}

#if ESP32
void handleSerialCommand(char *command, size_t length) {
    
    if (strncasecmp(command, "stats on", length) == 0) {
        bleHandler.SendStats();
    } else if (strncasecmp(command, "stats off", length) == 0) {
        bleHandler.StopStatsTimer();
    } else if (strncasecmp(command, "test", length) == 0) {
        char data[] = "short msg";
        //bleHandler.println(data);
        bleHandler.write((uint8_t *)data, strlen(data));
    } else if (strncasecmp(command, "test long", length) == 0) {
        char data[] = "this is super long message for testing transfer over BLE where is 20 chars limit per message";
        //bleHandler.println(data);
        bleHandler.write((uint8_t *)data, strlen(data));
    } else if (strncasecmp(command, "aux off", length) == 0) {
        auxSerialHandler.StopHeater(&handleHeaterComplete);
    } else if (strncasecmp(command, "aux on", length) == 0) {
        auxSerialHandler.LaunchHeater(&handleHeaterComplete);
    }
}
void handleBLEMessage(BinaryMessage * message) {
    handleSerialCommand((char *)message->data, (size_t)message->length);
}
#endif

void getBME280Data(BME280Data* data) {
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage
    if (data == NULL) return;

    data->temperature = bme280.readTemperature();
    data->humidity = bme280.readHumidity();
    data->pressure = bme280.readPressure();

}

void getBatteryData(BatteryData* data) {
    // response stats: STATS:in temp|out temp|humidity|pressure|voltage|ampers|calculated voltage
    if (data == NULL) return;

    // TODO:
    data->voltage = batteryMonitor.Voltage();
    data->ampers = 0;
    data->calcVoltage = 0;

}

void handleLevelChanged(VoltageLevelState level) {

    switch (level) {
    case VoltageLevelState::CRITICAL_LEVEL:
    case VoltageLevelState::DEAD_LEVEL:
    case VoltageLevelState::LOW_LEVEL:
    case VoltageLevelState::OVERFLOW_LEVEL:
        //gsmSerialHandler.SendSMSMessage(&handleLevelMessage);
        break;
    case VoltageLevelState::NORMAL_LEVEL:
    case VoltageLevelState::CHARGE_LEVEL:
        // DO Nothing, OK
        break;
    }
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

    DebugHandler::outWrite("handleDtmfCommand: ");
    DebugHandler::outWrite(code);

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
        stream->write(batteryMonitor.Voltage());
    }

    return result;
}
