#pragma once
#include "../common/Util.h"
#include "BaseSerialHandler.h"

class SerialCharResponseHandler: public BaseSerialHandler
{
private:
	char *separator;
	size_t separatorLength;

	size_t separatorMatchedLength = 0;

	void ResetBuffer();
	bool AppendSymbolToBuffer(uint8_t symbol);
	bool IsSeparatorRemainMatch(int remainSeparatorLength, uint8_t symbol);

protected:
	virtual bool LoadSymbolFromBuffer(uint8_t symbol);
    size_t maxBufferLength = 0;
	size_t bufferLength = 0;
	char *buffer = NULL;
public:
	SerialCharResponseHandler(const size_t bufferSize, const char *separator, Stream * serial);
	~SerialCharResponseHandler();

	void ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow = false) override;
	void Loop() override;
	bool IsBusy() override;

	void FlushData() override;
    bool IsLimitReached();
};

