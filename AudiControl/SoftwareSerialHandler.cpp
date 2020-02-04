#include "SoftwareSerialHandler.h"

SoftwareSerialHandler::SoftwareSerialHandler(uint8_t rxPin, uint8_t txPin):SoftwareSerial(rxPin, txPin)
{
	StartDeviceSerial(serialBaudRate);
}

SoftwareSerialHandler::~SoftwareSerialHandler()
{

}

void SoftwareSerialHandler::StartDeviceSerial(uint32_t serialBaudRate)
{
	this->serialBaudRate = serialBaudRate;

	end();
	begin(serialBaudRate);
}

void SoftwareSerialHandler::PrintSerialConfig()
{
	char pairity;
	switch (getParity())
	{
	case NONE:
		pairity = 'N';
		break;
	case ODD:
		pairity = 'O';
		break;
	case EVEN:
		pairity = 'E';
		break;
	}
	SerialPrintf("Baud: %d%c%d-%ld", GetDataBitsAmount(), pairity, GetStopBitsAmount(), serialBaudRate);
}

void SoftwareSerialHandler::SwitchSerialConfig(char *serialConfig)
{
	uint16_t type = 0;

	if (strncmp(serialConfig, "5N1", 3) == 0) type = CSERIAL_5N1;
	else if (strncmp(serialConfig, "6N1", 3) == 0) type = CSERIAL_6N1;
	else if (strncmp(serialConfig, "7N1", 3) == 0) type = CSERIAL_7N1;
	else if (strncmp(serialConfig, "8N1", 3) == 0) type = CSERIAL_8N1;

	else if (strncmp(serialConfig, "5N2", 3) == 0) type = CSERIAL_5N2;
	else if (strncmp(serialConfig, "6N2", 3) == 0) type = CSERIAL_6N2;
	else if (strncmp(serialConfig, "7N2", 3) == 0) type = CSERIAL_7N2;
	else if (strncmp(serialConfig, "8N2", 3) == 0) type = CSERIAL_8N2; // def

	else if (strncmp(serialConfig, "5E1", 3) == 0) type = CSERIAL_5E1;
	else if (strncmp(serialConfig, "6E1", 3) == 0) type = CSERIAL_6E1;
	else if (strncmp(serialConfig, "7E1", 3) == 0) type = CSERIAL_7E1;
	else if (strncmp(serialConfig, "8E1", 3) == 0) type = CSERIAL_8E1;

	else if (strncmp(serialConfig, "5E2", 3) == 0) type = CSERIAL_5E2;
	else if (strncmp(serialConfig, "6E2", 3) == 0) type = CSERIAL_6E2;
	else if (strncmp(serialConfig, "7E2", 3) == 0) type = CSERIAL_7E2;
	else if (strncmp(serialConfig, "8E2", 3) == 0) type = CSERIAL_8E2;

	else if (strncmp(serialConfig, "5O1", 3) == 0) type = CSERIAL_5O1;
	else if (strncmp(serialConfig, "6O1", 3) == 0) type = CSERIAL_6O1;
	else if (strncmp(serialConfig, "7O1", 3) == 0) type = CSERIAL_7O1;
	else if (strncmp(serialConfig, "8O1", 3) == 0) type = CSERIAL_8O1;

	else if (strncmp(serialConfig, "5O2", 3) == 0) type = CSERIAL_5O2;
	else if (strncmp(serialConfig, "6O2", 3) == 0) type = CSERIAL_6O2;
	else if (strncmp(serialConfig, "7O2", 3) == 0) type = CSERIAL_7O2;
	else if (strncmp(serialConfig, "8O2", 3) == 0) type = CSERIAL_8O2;

	if (type == 0) {
		// Wrong value!
		SerialPrintf("Unsupported %s", serialConfig);
		return;
	}
	else if (type == serialType)
	{
		PrintSerialConfig();
		return;
	}

	StartDeviceSerial(type, serialBaudRate);
	PrintSerialConfig();
}

void SoftwareSerialHandler::SwitchBaudRate(char *serialBaudRate)
{
	long baudRate = atol(serialBaudRate);

	StartDeviceSerial(serialType, baudRate);
}

uint8_t SoftwareSerialHandler::GetDataBitsAmount()
{
	return getNumberOfDataBit();
}
uint8_t SoftwareSerialHandler::GetStopBitsAmount()
{
	return getNumberOfStopBit();
}
uint32_t SoftwareSerialHandler::GetBaudRate()
{
	return serialBaudRate;
}


