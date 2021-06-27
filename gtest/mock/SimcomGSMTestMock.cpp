#include "SimcomGSMTestMock.h"

SimcomGSMTestMock::SimcomGSMTestMock(SerialStream * serialStream, SMSCallback smsCallback, DTMFCallback dtmfCallback):SimcomGSMHandler(smsCallback, dtmfCallback, serialStream)
{
	this->serialStream = serialStream;
}

SimcomGSMTestMock::~SimcomGSMTestMock()
{
}


void SimcomGSMTestMock::BeginInitialization()
{
	// AT Response
	ReadResponse((char *)"\r\nOK\r\n");

	// Sim state response
	ReadResponse((char *)"\r\n+CPIN: SIM PIN\r\n\r\nOK\r\n");

	// PIN input OK Response
	ReadResponse((char *)"\r\nOK\r\n");

	// Service response OK
	ReadResponse((char *)"\r\nSMS Ready\r\n");

	// Service response OK
	ReadResponse((char *)"\r\nCall Ready\r\n");

	ReadResponse((char *)"\r\n+CREG: 1,1\r\n");

}

void SimcomGSMTestMock::FinalizeInitialization()
{
	
	// Sim user list response
	ReadResponse((char *)"\r\n+CPBF: 1,\"+372111111\",145,\"1 aux-1\"\r\n");
	ReadResponse((char *)"\r\n+CPBF: 2,\"+372222222\",145,\"1 aux-2\"\r\n");
	ReadResponse((char *)"\r\nOK\r\n");
    
	ReadResponse((char *)"\r\n+CCLK: \"20/08/25,21:08:38+12\"\r\n");
	// Service response OK
	ReadResponse((char *)"\r\nOK\r\n");

}

void SimcomGSMTestMock::ReadResponse(char * response)
{
	Timer::Loop();
	this->Loop();

	serialStream->AddRXBuffer(response);

	while (serial->available() > 0) {
		Timer::Loop();
		this->Loop();
	}
}

void SimcomGSMTestMock::OnResponseReceived(bool IsTimeOut, bool isOverFlow)
{
	GSMSerialHandler::OnResponseReceived(IsTimeOut, isOverFlow);
}
