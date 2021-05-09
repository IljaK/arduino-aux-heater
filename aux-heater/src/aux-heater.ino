/*
 Name:    aux-heater.ino
 Created: 11/7/2018 4:04:00 PM
 Author:  Ilja
*/

#include "src/Definitions.h"
//#include "libs/ELECHOUSE_CC1101.h"
#include "src/common/Timer.h"
#include "src/common/Util.h"
#include "src/AuxHeaterSerial.h"
#include "src/BatteryMonitor.h"
#include "src/serial/DebugHandler.h"
#include <strings.h>
#include "src/bluetooth/BLEHandler.h"
#include "src/bluetooth/BluetoothSerialHandler.h"
#include "src/common/BinaryMessageStack.h"
#include "src/TemperatureHandler.h"
#include "src/common/Button.h"
#include "src/AudiControls.h"

void handleLevelChanged(VoltageLevelState level);
void handleSMSCommand(char* command, size_t size, time_t smsDispatchUTCts);
bool handleDtmfCommand(char code);
bool handleLevelMessage(Stream* stream);

TemperatureHandler temperatureHandler;
BatteryMonitor batteryMonitor(&handleLevelChanged);
AudiControls audiControls;

void handleSerialCommand(char *command, size_t length);

#if defined(ESP32)
    #include "src/gsm/SimomGSMHandler.h"

    HardwareSerial auxSerial(1);
    HardwareSerial gsmSerial(2);
    AuxHeaterSerial auxSerialHandler(&auxSerial);
    GSMSerialHandler gsmSerialHandler(&handleSMSCommand, &handleDtmfCommand, &gsmSerial);
    BLEHandler btHandler(handleSerialCommand);
#elif defined(MKRGSM1400)

    #define auxSerial Serial1
    
    Uart btSerial(&sercom3, BT_RX_PIN, BT_TX_PIN, SERCOM_RX_PAD_1, UART_TX_PAD_0);
    void SERCOM3_Handler() {
        btSerial.IrqHandler();
    }

    #include "src/gsm/UbloxGSMHandler.h"
    AuxHeaterSerial auxSerialHandler((HardwareSerial *)&auxSerial);
    UbloxGSMHandler gsmSerialHandler(handleSMSCommand, handleDtmfCommand, &SerialGSM);
    BluetoothSerialHandler btHandler(&btSerial, &handleSerialCommand);
#else
    #include "src/gsm/SimcomGSMHandler.h"
    //BluetoothSerialHandler btHandler(&Serial);
    AuxHeaterSerial auxSerialHandler(&Serial1);
    SimcomGSMHandler gsmSerialHandler(handleSMSCommand, handleDtmfCommand, &Serial2);
#endif

void setup() {
    // USB Serial
    Serial.begin(SERIAL_BAUD_RATE);
    //while(!Serial) {}

#if defined(ESP32)
    // AUX heater serial
    auxSerial.begin(AUX_BAUD_RATE, SERIAL_8N1, AUX_RX_PIN, AUX_TX_PIN);
    // gsm serial
    gsmSerial.begin(SERIAL_BAUD_RATE, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
    analogReadResolution(12);

#elif defined(MKRGSM1400)

    pinMode(GSM_RESETN, OUTPUT);
    digitalWrite(GSM_RESETN, LOW);
    pinMode(GSM_RTS, OUTPUT);
    pinMode(GSM_CTS, INPUT);
    digitalWrite(GSM_RTS, LOW);
    analogReadResolution(12);

    SerialGSM.begin(SERIAL_BAUD_RATE, SERIAL_8N1);
    auxSerial.begin(AUX_BAUD_RATE, SERIAL_8N1);

    btSerial.begin(BT_BAUD_RATE, SERIAL_8N1);
    pinPeripheral(BT_RX_PIN, PIO_SERCOM); // Assign RX function to pin 1
    pinPeripheral(BT_TX_PIN, PIO_SERCOM); // Assign TX function to pin 0
#endif

    //ledController.SetFrequency(100, 11, 0b00000001);

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

    audiControls.Start();
    temperatureHandler.Start();
    gsmSerialHandler.Start();
    batteryMonitor.Start();
    DebugHandler::outWrite("Setup done!\r\n");
}

void loop() {
    Timer::Loop();

    audiControls.Loop();

    //gsmSerialHandler.Loop();
    btHandler.Loop();

    //if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
    //	uint8_t size = ELECHOUSE_cc1101.ReceiveData(RX_buffer, CC1101_BUFFER_SIZE);
    //
    //	DebugHandler::outWrite(F("CC1101: "));
    //	DebugHandler::PrintBytes(RX_buffer, size);
    //	DebugHandler::outWriteEnd();
    //}

    //if (Serial && Serial.available() > 0) {
    //    while (Serial && Serial.available() > 0) {
    //        SerialGSM.write(Serial.read());
    //    }
    //}

    // Must be latest task loop
    TimeManager::LateLoop();
}

void handleSerialCommand(char *command, size_t length) {

    //Serial.print("handleSerialCommand: ");
    //Serial.print(command);
    //Serial.println();

    if (strncasecmp(command, "test", length) == 0) {
        char data[] = "short msg";
        //bleHandler.println(data);
        btHandler.write((uint8_t *)data, strlen(data));
    } else if (strncasecmp(command, "test long", length) == 0) {
        char data[] = "this is super long message for testing transfer over BLE where is 20 chars limit per message";
        //bleHandler.println(data);
        btHandler.write((uint8_t *)data, strlen(data));
    } else if (strncasecmp(command, "aux off", length) == 0) {
        //auxSerialHandler.StopHeater(&handleHeaterComplete);
    } else if (strncasecmp(command, "aux on", length) == 0) {
        //auxSerialHandler.LaunchHeater(&handleHeaterComplete);
    }
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
        stream->write(batteryMonitor.CalcVoltage());
    }

    return result;
}

void handleEmergencyClick(uint8_t multiClicks) {
    DebugHandler::outWrite("handleEmergencyClick ");
    DebugHandler::outWriteASCII(multiClicks);
    DebugHandler::outWriteEnd();
}
void handleServiceClick(uint8_t multiClicks) {
    DebugHandler::outWrite("handleServiceClick ");
    DebugHandler::outWriteASCII(multiClicks);
    DebugHandler::outWriteEnd();
}
