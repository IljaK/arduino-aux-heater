#pragma once
#include "common/Util.h"
#include "serial/SerialTimerResponseHandler.h"

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

