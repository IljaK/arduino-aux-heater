#include <gtest/gtest.h>
#include "mock/SerialCharResponseHandlerMock.h"
#include "mock/BaseSerialHandlerMock.h"
#include "mock/GSMSerialHandlerTestMock.h"
#include <Arduino.h>
#include "SerialStream.h"
#include <BaseSerialHandler.h>
#include "mock/TimerMock.h"

void respondSerial(char * response, SerialStream *serial, BaseSerialHandler * handler)
{
	Timer::Loop();
	handler->Loop();

	serial->AddRXBuffer(response);

	while (serial->available() > 0) {
		Timer::Loop();
		handler->Loop();
	}
}

bool messageCallback(Stream *stream)
{
	stream->write((char*)"Simple text");
	return true;
}

TEST(GSMSerialHandlerTestMock, ATResponseTest)
{
	// TODO;
	timeOffset = 0;
	TimerMock::Reset();
	
	SerialStream serial;
	GSMSerialHandlerTestMock gsmHandler(&serial);

	Timer::Loop();
	gsmHandler.Loop();

	// AT Response
	respondSerial((char *)"\r\nAT\r\n\r\nOK\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::SHORT_RESPONSE);

	// ATE0 Response
	respondSerial((char *)"\r\nOK\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::REG_GSM_SERVICE);

	// +CREG Response
	respondSerial((char *)"\r\nOK\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::SIM_PIN_STATE);

	// Sim state response
	respondSerial((char *)"\r\n+CPIN: SIM PIN\r\n\r\nOK\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::SIM_LOGIN);

	// PIN input OK Response
	respondSerial((char *)"\r\nOK\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::WAIT_SIM_INIT);

	// Service response OK
	respondSerial((char *)"\r\nSMS Ready\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::MSG_TEXT_MODE);

	// Msg text mode
	respondSerial((char *)"\r\nOK\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::MSG_INCOMING_FORMAT);

	// Msg text format
	respondSerial((char *)"\r\nOK\r\n", &serial, &gsmHandler);
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::FIND_PRIMARY_PHONE);

	respondSerial((char *)"\r\n+CPBF: 1,\"+372111111\",145,\"1 aux-1\"\r\n", &serial, &gsmHandler);
	respondSerial((char *)"\r\n+CPBF: 2,\"+372222222\",145,\"1 aux-2\"\r\n", &serial, &gsmHandler);
	respondSerial((char *)"\r\nOK\r\n", &serial, &gsmHandler);

	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

	gsmHandler.SendSMSMessage(&messageCallback);

	char smsTextResponse[] = "\r\n> ";
	respondSerial(smsTextResponse, &serial, &gsmHandler);

	respondSerial((char *)"\r\nOK\r\n", &serial, &gsmHandler);
	
	EXPECT_EQ(gsmHandler.FlowState(), GSMFlowState::READY);

}