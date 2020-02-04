#pragma once
#include <Arduino.h>
#include "Util.h"
#include "SerialCharResponseHandler.h"
#include "SerialTimerResponseHandler.h"
#include "AuxHeaterSerial.h"

constexpr char SIM_PIN_CODE[] = "0000"; // Pin code for sim card
//constexpr char SIM_PIN_CODE[] = "58349965";
//constexpr char SIM_PIN_CODE[] = "4362";

constexpr char GSM_RESPONSE_SEPARATOR[] = "\r\n";
constexpr char GSM_OK_RESPONSE[] = "OK";
constexpr char GSM_ERROR_RESPONSE[] = "ERROR";
constexpr char GSM_INIT_CMD[] = "AT";

constexpr uint8_t GSM_CMD_SET_SYMBOL = '=';
constexpr uint8_t GSM_CMD_ASK_SYMBOL = '?';
constexpr uint8_t COMMA_ASCII_SYMBOL = ',';

constexpr uint8_t CR_ASCII_SYMBOL = 13u; // CR
constexpr uint8_t LF_ASCII_SYMBOL = 10u; // LF
constexpr uint8_t CRTLZ_ASCII_SYMBOL = 26u; // ctrl+z
constexpr uint8_t ESC_ASCII_SYMBOL = 27u; // ESC

// Finalize responses:
// OK
// +CME ERROR:
// +CMS ERROR:

// AT+CMGF=1 - set sms in text mode
// AT+CPBR=1 - get number from sim card
// AT+CNMI=1,2,0,0,0 - print new arrived messages

constexpr char GSM_SHORT_RESPONSE[] = "E0"; // Disable request in response
//constexpr char GSM_SIM_ICCID[] = "+QCCID"; // Get sim ccid
constexpr char GSM_SIGNAL_CMD[] = "+CSQ"; // Signal quality test, value range is 0-31 , 31 is the best
constexpr char GSM_SIM_PIN_CMD[] = "+CPIN"; // Rad sim pin code state
constexpr char GSM_REG_CMD[] = "+CREG"; // set 1 for sim registration

// AT+CPBS="SM" - set phonebook sim card

constexpr char GSM_MSG_TEXT_CMD[] = "+CMGF"; // Force msg text mode
constexpr char GSM_MSG_ARRIVE_CMD[] = "+CNMI"; // Message arrive event "AT+CNMI=1,2,0,0,0"
constexpr char GSM_SMS_SEND_CMD[] = "+CMGS"; // Send sms message
constexpr char GSM_FIND_USER_CMD[] = "+CPBF"; // Find phonebook entries

//constexpr char GSM_EVENT_DATA_CMD[] = "+CIEV"; // Data event
constexpr char GSM_TS_DATA_CMD[] = "+CMT"; // Data event
constexpr char GSM_EVENT_DATA_MESSAGE[] = "MESSAGE"; // Data event
constexpr char GSM_EVENT_DATA_SERVICE[] = "service"; // Data event
constexpr char GSM_EVENT_DATA_ROAMING[] = "roam"; // Data event


constexpr char GSM_AUX_ENABLE[] = "on"; // Data event
constexpr char GSM_AUX_DISABLE[] = "off"; // Data event
constexpr char GSM_AUX_PHONE_POSTFIX[] = "aux-"; // Data event

//constexpr char GSM_REG_CMD[] = "+CREG?"; // Check whether it has registered in the network
//constexpr char GSM_SMS_TEXT_MODE[] = "AT+CMGF=1\r";

constexpr char GSM_SIM_AUTH_READY[] = "SMS Ready";
constexpr char GSM_SIM_STATE_READY[] = "READY"; // Pin code for sim card
constexpr char GSM_SIM_STATE_SIM_PIN[] = "SIM PIN"; // Pin code for sim card

enum class GSMFlowState : uint8_t
{
	INITIALIZATION,
	SHORT_RESPONSE,
	//CHECK_SIM_CCID,
	REG_GSM_SERVICE,

	SIM_PIN_STATE,

	SIM_PIN_STATE_READY,
	SIM_PIN_STATE_PIN,
	SIM_PIN_STATE_UNKNOWN,

	SIM_LOGIN,

	WAIT_SIM_INIT,

	MSG_TEXT_MODE,
	MSG_INCOMING_FORMAT,

	FIND_PRIMARY_PHONE,

	AUTH_INCOMING_MSG,
	NOT_AUTH_INCOMING_MSG,

	SEND_SMS_BEGIN,
	SEND_SMS_FLOW,

	READY,
	LOCKED

};

class GSMSerialHandler : public SerialCharResponseHandler //SerialTimerResponseHandler
{
private:
	Timer flowTimer;

	char primaryPhone[16] = "+37258349965";
	uint8_t lowestIndex = 255;

	char smsSender[16];

	GSMFlowState flowState = GSMFlowState::INITIALIZATION;
	bool tryedPass = false;
	uint8_t cRegState = 0;

	StringCallback smsCallback;
	StreamCallback messageCallback;

	bool IsProperResponse(char *response, size_t size);
	void LaunchStateRequest();

	void HandleErrorResponse(char *response, size_t size);
	void HandleDataResponse(char *response, size_t size);
	void HandleOKResponse(char *response, size_t size);

	void HandleIncomingMessage(char *response, size_t size);
	void FinalizeSendMessage();

public:
	GSMSerialHandler(StringCallback smsCallback, Stream * serial);
	~GSMSerialHandler();

	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false) override;
	void Loop() override;
	bool IsBusy() override;
	void SendSMSMessage(StreamCallback messageCallback);

	bool IsNetworkConnected();
	bool IsRoaming();

};

