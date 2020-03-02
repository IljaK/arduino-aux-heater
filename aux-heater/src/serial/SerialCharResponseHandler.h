#pragma once
#include "../common/Util.h"
#include "BaseSerialHandler.h"

class SerialCharResponseHandler: public BaseSerialHandler
{
private:
	char *separator;
	size_t separatorLength;

	size_t separatorMatchedLength = 0;

	bool LoadSymbolFromBuffer(uint8_t symbol);
	void ResetBuffer();
	bool AppendSymbolToBuffer(uint8_t symbol);
	bool IsSeparatorRemainMatch(int remainSeparatorLength, uint8_t symbol);

protected:
	size_t bufferLength = 0;
	char buffer[SERIAL_RX_BUFFER_SIZE];
public:
	SerialCharResponseHandler(const char *separator, Stream * serial);
	~SerialCharResponseHandler();

	void ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow = false) override;
	void Loop() override;
	bool IsBusy() override;

	void FlushData() override;
};

