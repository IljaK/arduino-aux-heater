#pragma once
#include <Arduino.h>
#include "../common/Util.h"
#include "../common/TimeManager.h"
#include "../serial/SerialCharResponseHandler.h"
#include "../serial/SerialTimerResponseHandler.h"
#include "../common/StringStackArray.h"
#include "../common/DebugHandler.h"

constexpr char SIM_PIN_CODE[] = "0000"; // Pin code for sim card

constexpr char GSM_OK_RESPONSE[] = "OK";
constexpr char GSM_ERROR_RESPONSE[] = "ERROR";
constexpr char GSM_INIT_CMD[] = "AT";

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
// AT+CLCC=1 // Call status report
// AT+DDET=1 // DTFM signal detection

// AT+CSMINS - Sim card insert status

// U-blox special commands
// AT+CLIP=1 // Enable caller number display
// +CLIP: "+372000000",145,,,,0
// AT+CLCC // get pending call info
// AT+CTZU - automatic time zone update
// AT+UCALLSTAT=1 // Enable call state events

//constexpr char GSM_SIM_ICCID[] = "+QCCID"; // Get sim ccid
//constexpr char GSM_SIGNAL_CMD[] = "+CSQ"; // Signal quality test, value range is 0-31 , 31 is the best
constexpr char GSM_SIM_PIN_CMD[] = "+CPIN"; // Read sim pin code state
constexpr char GSM_REG_CMD[] = "+CREG"; // set 1 for sim registration
constexpr char GSM_TIME_CMD[] = "+CCLK"; // Time request
constexpr char GSM_SWITCH_CMD[] = "+CFUN"; // Call state

// AT+CPBS="SM" - set phonebook sim card
constexpr char GSM_SMS_SEND_CMD[] = "+CMGS"; // Send sms message
constexpr char GSM_MSG_TEXT_INPUT_RESPONSE[] = "> "; // Response after sucess send cmd
constexpr char GSM_FIND_USER_CMD[] = "+CPBF"; // Find phonebook entries

//constexpr char GSM_EVENT_DATA_CMD[] = "+CIEV"; // Data event
constexpr char GSM_TS_DATA_CMD[] = "+CMT"; // Data event

constexpr char GSM_CALL_DIAL_CMD[] = "D"; // ATD Call dial cmd
constexpr char GSM_CALL_HANGUP_CMD[] = "H"; // ATH Call hangup cmd 
constexpr char GSM_CALL_ANSWER_CMD[] = "A"; // ATA Call answer cmd 
constexpr char GSM_CALL_STATE_CMD[] = "+CLCC"; // Call state
constexpr char GSM_DTMF_CMD[] = "+DTMF"; // Call state

constexpr char GSM_AUX_PHONE_POSTFIX[] = "aux-"; // Data event

constexpr char GSM_SIM_STATE_READY[] = "READY"; // Ready sim card state
constexpr char GSM_SIM_STATE_SIM_PIN[] = "SIM PIN"; // Pin code input sim card state

constexpr uint32_t CALL_HANGUP_DELAY = 5000000U; // 5 seconds
constexpr uint32_t CALL_ANSWER_DELAY = 500000U; // 0.5 seconds

constexpr uint8_t SMS_CALL_HANGUP_SIZE = 2;
constexpr uint32_t GSM_CALL_DELAY = 1000000U;

enum GSMSimPinState:uint8_t
{
	SIM_PIN_STATE_UNKNOWN,
	SIM_PIN_STATE_READY,
	SIM_PIN_STATE_PIN,
	SIM_PIN_STATE_ERROR
};

enum GSMSMSState : uint8_t
{
	NONE,

	RECEIVE_AUTH,
	RECEIVE_NOT_AUTH,

    SEND_BEGIN,
    SEND_FLOW
};

enum GSMCallState : uint8_t
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

enum CallTimerState:uint8_t
{
    DIAL,
    HANGUP,
    ANSWER
};

typedef void (*SMSCallback)(char *message, size_t size, time_t timeStamp);
typedef bool (*DTMFCallback)(char code);

class GSMSerialHandler : public SerialCharResponseHandler
{
private:
	TimerID flowTimer = 0;
	TimerID callTimer = 0;
	//TimerID callDelayTimer = 0;

	char smsSender[18];
	time_t smsDispatchUTCts = 0;

    char *reqCmd = NULL;
	StringStackArray callHangupStack = StringStackArray(SMS_CALL_HANGUP_SIZE);

	GSMSimPinState simPinState = GSMSimPinState::SIM_PIN_STATE_UNKNOWN;
	GSMSMSState smsState = GSMSMSState::NONE;
	GSMCallState callState = GSMCallState::DISCONNECT;

	int cRegState = 0;

	SMSCallback smsCallback;
	DTMFCallback dtmfCallback;
	StreamCallback messageCallback;

	bool IsProperResponse(char *response, size_t size);

	void FinalizeSendMessage();

	void HangupCallCMD();

	void CallCMD();
	void AnswerCallCMD();
	void StopCallTimer();

protected:

    void WriteGsmSerial(const char * cmd, bool isCheck = false, bool isSet = false, char *data = NULL, bool dataQuotations = false, bool semicolon = false);

	bool LoadSymbolFromBuffer(uint8_t symbol) override;
	virtual void HandleErrorResponse(char * reqCmd, char *response, size_t size);
	virtual void HandleDataResponse(char * reqCmd, char *response, size_t size);
	virtual void HandleOKResponse(char * reqCmd, char *response, size_t size);
	virtual bool IsAuthorized(char *number, char *entryName) = 0;
    virtual void OnNetworkStateUpdated() = 0;
    virtual void OnFlowTimer() = 0;
    virtual void OnCMDRequest(char * reqCmd);

	void StartFlowTimer(unsigned long duration);

public:
	GSMSerialHandler(SMSCallback smsCallback, DTMFCallback dtmfCallback, Stream * serial);
	virtual ~GSMSerialHandler();

	void OnTimerComplete(TimerID timerId, uint8_t data) override;
	void OnResponseReceived(bool isTimeOut, bool isOverFlow = false) override;
	bool IsBusy() override;
	void SendSMSMessage(StreamCallback messageCallback, char * phone);
	void NotifyByCallHangUp();

    virtual void Start() = 0;

	bool IsNetworkConnected();
	bool IsRoaming();

	GSMCallState CallState();
	GSMSMSState SMSState();
    GSMSimPinState SimPinState();

    char *PendingRequest();
};

