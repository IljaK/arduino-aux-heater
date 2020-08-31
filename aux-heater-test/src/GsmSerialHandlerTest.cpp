#include <gtest/gtest.h>
#include "mock/SerialCharResponseHandlerMock.h"
#include "mock/BaseSerialHandlerMock.h"
#include "mock/GSMSerialHandlerTestMock.h"
#include <Arduino.h>
#include "mock/SerialStream.h"
#include <BaseSerialHandler.h>
#include "mock/TimerMock.h"
#include <StackArray.h>

bool messageInputCallBack(Stream *stream)
{
	stream->write((char*)"Simple text");
	return true;
}

TEST(GSMSerialHandlerTest, GSMInitializeTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	GSMSerialHandlerTestMock gsmHandler(&serial);

	Timer::Loop();
	gsmHandler.Loop();

	// AT Response
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::SIM_PIN_STATE);

	// Sim state response
	gsmHandler.ReadResponse((char *)"\r\n+CPIN: SIM PIN\r\n\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::SIM_LOGIN);

	// Sim input response
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::WAIT_SIM_INIT);

	gsmHandler.ReadResponse((char *)"\r\nSMS Ready\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::WAIT_SIM_INIT);

	gsmHandler.ReadResponse((char *)"\r\nCall Ready\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::TIME_REQUEST);

	gsmHandler.ReadResponse((char *)"\r\n+CCLK: \"20/08/25,21:08:38+12\"\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::TIME_REQUEST);

	// Service response OK
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::FIND_PRIMARY_PHONE);

	// Sim user list response
	gsmHandler.ReadResponse((char *)"\r\n+CPBF: 1,\"+372111111\",145,\"1 aux-1\"\r\n");
	gsmHandler.ReadResponse((char *)"\r\n+CPBF: 2,\"+372222222\",145,\"1 aux-2\"\r\n");
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");

	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);
}

TEST(GSMSerialHandlerTest, LongSMSTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	GSMSerialHandlerTestMock gsmHandler(&serial);

	gsmHandler.BeginInitialization();
	gsmHandler.FinalizeInitialization();

	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);


	// Send SMS logic test
	gsmHandler.SendSMSMessage(&messageInputCallBack);

	char smsTextResponse[] = "\r\n> ";
	gsmHandler.ReadResponse(smsTextResponse);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

	// Incoming SMS logic test + save sender number
	gsmHandler.ReadResponse((char *)"\r\n+CMT: \"+372111111\",\"ilja aux-1\",\"20/08/27,00:25:23+12\"\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

	// Long sms
	gsmHandler.ReadResponse((char *)"Test super long sms: kgkdkgre[wpegj'oewg'poergprelfjwelo fjwoej");
	EXPECT_EQ(gsmHandler.SMSState(), GSMIncomingMessageState::AUTH_INCOMING_MSG);

	gsmHandler.ReadResponse((char *)"fewlqg;wglweg\r\n");
	EXPECT_EQ(gsmHandler.SMSState(), GSMIncomingMessageState::NONE);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);
}

TEST(GSMSerialHandlerTest, CallHangupTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	GSMSerialHandlerTestMock gsmHandler(&serial);

	gsmHandler.BeginInitialization();
	gsmHandler.FinalizeInitialization();

	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);


	// Send SMS logic test
	gsmHandler.SendSMSMessage(&messageInputCallBack);

	char smsTextResponse[] = "\r\n> ";
	gsmHandler.ReadResponse(smsTextResponse);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

	// Call initialized
	gsmHandler.ReadResponse((char *)"\r\n+CLCC: 1,0,2,0,0,\"+3721111111\",145,\"241 aux-1\"\r\n");
	EXPECT_EQ(gsmHandler.CallState(), GSMCallState::DIALING);
	// Call established
	gsmHandler.ReadResponse((char *)"\r\n+CLCC: 1,0,3,0,0,\"+3721111111\",145,\"241 aux-1\"\r\n");
	EXPECT_EQ(gsmHandler.CallState(), GSMCallState::ALERTING);

	timeOffset += CALL_HANGUP_DELAY;
	Timer::Loop();
	gsmHandler.Loop();

	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::CALL_HANGUP);
	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

	// Call ended
	gsmHandler.ReadResponse((char *)"\r\n+CLCC: 1,0,6,0,0,\"+3721111111\",145,\"241 aux-1\"\r\n");
	EXPECT_EQ(gsmHandler.CallState(), GSMCallState::DISCONNECT);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);
}

TEST(GSMSerialHandlerTest, SMSCallHangupTest)
{
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	GSMSerialHandlerTestMock gsmHandler(&serial);

	gsmHandler.BeginInitialization();
	gsmHandler.FinalizeInitialization();

	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);


	// Send SMS logic test
	gsmHandler.SendSMSMessage(&messageInputCallBack);

	char smsTextResponse[] = "\r\n> ";
	gsmHandler.ReadResponse(smsTextResponse);

	gsmHandler.ReadResponse((char *)"\r\nOK\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

	// Incoming SMS logic test + save sender number
	gsmHandler.ReadResponse((char *)"\r\n+CMT: \"+372111111\",\"ilja aux-1\",\"20/08/27,00:25:23+12\"\r\n");
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);
	EXPECT_EQ(gsmHandler.SMSState(), GSMIncomingMessageState::AUTH_INCOMING_MSG);

	// SMS with command
	gsmHandler.ReadResponse((char *)"On\r\n");
	EXPECT_EQ(gsmHandler.SMSState(), GSMIncomingMessageState::NONE);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

	gsmHandler.NotifyByCallHangUp();
	gsmHandler.Loop();
	timeOffset+= GSM_CALL_DELAY;
	gsmHandler.Loop();
	Timer::Loop();
	gsmHandler.Loop();
}