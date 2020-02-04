#pragma once
#include "Util.h"
#include "SerialTimerResponseHandler.h"

class CMDSerialHandler: public SerialTimerResponseHandler
{
private:
	StringArrayCallback commandCallback = NULL;
protected:
	void OnResponseReceived(bool IsTimeOut, bool isOverFlow = false) override;
public:
	CMDSerialHandler(Stream* stream, StringArrayCallback commandCallback = NULL);
	~CMDSerialHandler();
	void Loop() override;
};

