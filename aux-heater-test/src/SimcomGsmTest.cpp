#include <gtest/gtest.h>
#include "gsm/SimcomGSMHandler.h"
#include "mock/SimcomGSMTestMock.h"
#include <Arduino.h>
#include "mock/SerialStream.h"
#include "mock/TimerMock.h"

bool messageInputCallBack(Stream *stream)
{
	stream->write((char*)"Simple text");
	return true;
}

TEST(SimcomGsmTest, GSMInitializeTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	SimcomGSMTestMock gsmHandler(&serial);

	Timer::Loop();
	gsmHandler.Loop();

	// AT Response
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::SIM_PIN_STATE);

	// Sim state response
	gsmHandler.ReadResponse((char *)"\r\n+CPIN: SIM PIN\r\n\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::SIM_LOGIN);

	// Sim input response
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::REG_NETWORK);

	gsmHandler.ReadResponse((char *)"\r\nSMS Ready\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::REG_NETWORK);

	gsmHandler.ReadResponse((char *)"\r\nCall Ready\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::REG_NETWORK);

	gsmHandler.ReadResponse((char *)"\r\n+CREG: 1,1\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::FIND_PRIMARY_PHONE);

	// Sim user list response
	gsmHandler.ReadResponse((char *)"\r\n+CPBF: 1,\"+372111111\",145,\"1 aux-1\"\r\n");
	gsmHandler.ReadResponse((char *)"\r\n+CPBF: 2,\"+372222222\",145,\"1 aux-2\"\r\n");
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::TIME_REQUEST);

	gsmHandler.ReadResponse((char *)"\r\n+CCLK: \"20/08/25,21:08:38+12\"\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::TIME_REQUEST);

	// Service response OK
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);
}

TEST(SimcomGsmTest, LongSMSTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	SimcomGSMTestMock gsmHandler(&serial);

	gsmHandler.BeginInitialization();
	gsmHandler.FinalizeInitialization();

	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);

	// Send SMS logic test
	gsmHandler.SendSMSMessage(&messageInputCallBack);

	char smsTextResponse[] = "\r\n> ";
	gsmHandler.ReadResponse(smsTextResponse);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);

	// Incoming SMS logic test + save sender number
	gsmHandler.ReadResponse((char *)"\r\n+CMT: \"+372111111\",\"ilja aux-1\",\"20/08/27,00:25:23+12\"\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);

	// Long sms
	gsmHandler.ReadResponse((char *)"Test super long sms: kgkdkgre[wpegj'oewg'poergprelfjwelo fjwoej");
	EXPECT_EQ(gsmHandler.SMSState(), GSMSMSState::RECEIVE_AUTH);

	gsmHandler.ReadResponse((char *)"fewlqg;wglweg\r\n");
	EXPECT_EQ(gsmHandler.SMSState(), GSMSMSState::NONE);
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);
}

TEST(SimcomGsmTest, CallHangupTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	SimcomGSMTestMock gsmHandler(&serial);

	gsmHandler.BeginInitialization();
	gsmHandler.FinalizeInitialization();

	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);


	// Send SMS logic test
	gsmHandler.SendSMSMessage(&messageInputCallBack);

	char smsTextResponse[] = "\r\n> ";
	gsmHandler.ReadResponse(smsTextResponse);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);

	// Call initialized
	gsmHandler.ReadResponse((char *)"\r\n+CLCC: 1,0,2,0,0,\"+3721111111\",145,\"241 aux-1\"\r\n");
	EXPECT_EQ(gsmHandler.CallState(), GSMCallState::DIALING);
	// Call established
	gsmHandler.ReadResponse((char *)"\r\n+CLCC: 1,0,3,0,0,\"+3721111111\",145,\"241 aux-1\"\r\n");
	EXPECT_EQ(gsmHandler.CallState(), GSMCallState::ALERTING);

	timeOffset += CALL_HANGUP_DELAY;
	Timer::Loop();
	gsmHandler.Loop();

	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);

	// Call ended
	gsmHandler.ReadResponse((char *)"\r\n+CLCC: 1,0,6,0,0,\"+3721111111\",145,\"241 aux-1\"\r\n");
	EXPECT_EQ(gsmHandler.CallState(), GSMCallState::DISCONNECT);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);
}

TEST(SimcomGsmTest, SMSCallHangupTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	SimcomGSMTestMock gsmHandler(&serial);

	gsmHandler.BeginInitialization();
	gsmHandler.FinalizeInitialization();

	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);


	// Send SMS logic test
	gsmHandler.SendSMSMessage(&messageInputCallBack);

	char smsTextResponse[] = "\r\n> ";
	gsmHandler.ReadResponse(smsTextResponse);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);

	// Incoming SMS logic test + save sender number
	gsmHandler.ReadResponse((char *)"\r\n+CMT: \"+372111111\",\"ilja aux-1\",\"20/08/27,00:25:23+12\"\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);
	EXPECT_EQ(gsmHandler.SMSState(), GSMSMSState::RECEIVE_AUTH);

	// SMS with command
	gsmHandler.ReadResponse((char *)"On\r\n");
	EXPECT_EQ(gsmHandler.SMSState(), GSMSMSState::NONE);
	EXPECT_EQ(gsmHandler.FlowState(), SimcomFlowState::READY);

	gsmHandler.NotifyByCallHangUp();
	gsmHandler.Loop();
	timeOffset+= GSM_CALL_DELAY;
	gsmHandler.Loop();
	Timer::Loop();
	gsmHandler.Loop();
}