#pragma once
#include <Arduino.h>
#include "time.h"
#include "common/Util.h"
#include "serial/SerialCharResponseHandler.h"
#include "serial/SerialTimerResponseHandler.h"
#include "common/StringStackArray.h"

constexpr char SIM_PIN_CODE[] = "0000"; // Pin code for sim card

constexpr char GSM_RESPONSE_SEPARATOR[] = "\r\n";
constexpr char GSM_OK_RESPONSE[] = "OK";
constexpr char GSM_ERROR_RESPONSE[] = "ERROR";
constexpr char GSM_INIT_CMD[] = "AT";

constexpr uint8_t GSM_CMD_SET_SYMBOL = '=';
constexpr uint8_t GSM_CMD_ASK_SYMBOL = '?';
constexpr uint8_t COMMA_ASCII_SYMBOL = ',';
constexpr uint8_t SEMICOLON_ASCII_SYMBOL = ';';

constexpr uint8_t CR_ASCII_SYMBOL = 13u; // CR
constexpr uint8_t LF_ASCII_SYMBOL = 10u; // LF
constexpr uint8_t CRTLZ_ASCII_SYMBOL = 26u; // ctrl+z
constexpr uint8_t ESC_ASCII_SYMBOL = 27u; // ESC

// Finalize responses:
// OK
// +CME ERROR:
// +CMS ERROR:

// AT+CPBR=1 - get number from sim card

// Saved variables for sim800:
// ATE0
// AT+CREG=1
// AT+CMGF=1
// AT+CNMI=2,2,0,0,0
// AT+CLCC=1
// AT+DDET=1
// AT+CLIP=0

// AT+CSMINS - Sim card insert status

//constexpr char GSM_SIM_ICCID[] = "+QCCID"; // Get sim ccid
//constexpr char GSM_SIGNAL_CMD[] = "+CSQ"; // Signal quality test, value range is 0-31 , 31 is the best
constexpr char GSM_SIM_PIN_CMD[] = "+CPIN"; // Read sim pin code state
constexpr char GSM_REG_CMD[] = "+CREG"; // set 1 for sim registration
constexpr char GSM_TIME_CMD[] = "+CCLK"; // set 1 for sim registration

// AT+CPBS="SM" - set phonebook sim card
constexpr char GSM_SMS_SEND_CMD[] = "+CMGS"; // Send sms message
constexpr char GSM_MSG_TEXT_INPUT_RESPONSE[] = "> "; // Response after sucess send cmd
constexpr char GSM_FIND_USER_CMD[] = "+CPBF"; // Find phonebook entries

//constexpr char GSM_EVENT_DATA_CMD[] = "+CIEV"; // Data event
constexpr char GSM_TS_DATA_CMD[] = "+CMT"; // Data event

constexpr char GSM_CALL_DIAL_CMD[] = "ATD"; // Call dial cmd
constexpr char GSM_CALL_HANGUP_CMD[] = "ATH"; // Call hangup cmd 
constexpr char GSM_CALL_ANSWER_CMD[] = "ATA"; // Call answer cmd 
constexpr char GSM_CALL_STATE_CMD[] = "+CLCC"; // Call state
constexpr char GSM_DTMF_CMD[] = "+DTMF"; // Call state

constexpr char GSM_AUX_ENABLE_DTFM = '1'; // Data event
constexpr char GSM_AUX_DISABLE_DTFM = '0'; // Data event

constexpr char GSM_AUX_ENABLE[] = "on"; // Data event
constexpr char GSM_AUX_DISABLE[] = "off"; // Data event
constexpr char GSM_AUX_PHONE_POSTFIX[] = "aux-"; // Data event

//constexpr char GSM_REG_CMD[] = "+CREG?"; // Check whether it has registered in the network
//constexpr char GSM_SMS_TEXT_MODE[] = "AT+CMGF=1\r";

constexpr char GSM_SIM_AUTH_READY[] = "SMS Ready";
constexpr char GSM_SIM_STATE_READY[] = "READY"; // Pin code for sim card
constexpr char GSM_SIM_STATE_SIM_PIN[] = "SIM PIN"; // Pin code for sim card

constexpr uint32_t CALL_HANGUP_DELAY = 5000000U; // 5 seconds
constexpr uint32_t CALL_ANSWER_DELAY = 500000U; // 0.5 seconds

constexpr uint8_t SMS_CALL_HANGUP_SIZE = 2;
constexpr uint32_t GSM_CALL_DELAY = 1000000U;

enum class GSMFlowState : uint8_t
{
	INITIALIZATION,

	SIM_PIN_STATE,
	SIM_PIN_STATE_READY,
	SIM_PIN_STATE_PIN,
	SIM_PIN_STATE_UNKNOWN,

	SIM_LOGIN,
	WAIT_SIM_INIT,

	TIME_REQUEST,
	FIND_PRIMARY_PHONE,

	READY,
	LOCKED,

	// SMS sending states
	SEND_SMS_BEGIN,
	SEND_SMS_FLOW,

	// Call action cmd
	CALL_DIAL,
	CALL_HANGUP,
	CALL_ANSWER
};

enum class IncomingMessageState : uint8_t
{
	NONE,
	AUTH_INCOMING_MSG,
	NOT_AUTH_INCOMING_MSG
};

enum class GSMCallState : uint8_t
{
	ACTIVE,
	HELD,

	// Outgoing
	DIALING,
	ALERTING,

	// Incoming
	INCOMING,
	WAITING,

	DISCONNECT
};

typedef void (*SMSCallback)(char *message, size_t size, time_t timeStamp);
typedef bool (*DTMFCallback)(char code);

class GSMSerialHandler : public SerialCharResponseHandler
{
private:
	TimerID flowTimer = 0;
	TimerID callTimer = 0;
	TimerID callDelayTimer = 0;

	char primaryPhone[18] = "";
	uint8_t lowestIndex = 255;

	char smsSender[18];
	time_t smsSendTS = 0;

	StringStackArray callHangupStack = StringStackArray(SMS_CALL_HANGUP_SIZE);

	GSMFlowState flowState = GSMFlowState::INITIALIZATION;
	IncomingMessageState smsState = IncomingMessageState::NONE;
	GSMCallState callState = GSMCallState::DISCONNECT;

	bool tryedPass = false;
	uint8_t cRegState = 0;

	SMSCallback smsCallback;
	DTMFCallback dtmfCallback;
	StreamCallback messageCallback;

	bool IsProperResponse(char *response, size_t size);
	void LaunchFlowRequest();

	void HandleErrorResponse(char *response, size_t size);
	void HandleDataResponse(char *response, size_t size);
	void HandleOKResponse(char *response, size_t size);

	void FinalizeSendMessage();

	void StartFlowTimer(unsigned long duration);
	void HangupCallCMD();

	void CallCMD();
	void AnswerCallCMD();
	void StopCallTimer();
	void StartCallDelayTimer();

protected:
	bool LoadSymbolFromBuffer(uint8_t symbol) override;
public:
	GSMSerialHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial);
	~GSMSerialHandler();

	void OnTimerComplete(TimerID timerId) override;
	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false);
	bool IsBusy() override;
	void SendSMSMessage(StreamCallback messageCallback);
	void NotifyByCallHangUp();

	bool IsNetworkConnected();
	bool IsRoaming();
	bool IsAuthorized(char *entryName);

	GSMFlowState FlowState();
	GSMCallState CallState();
	IncomingMessageState SMSState();
};

