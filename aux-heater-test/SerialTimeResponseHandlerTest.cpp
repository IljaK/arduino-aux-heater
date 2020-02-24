#include "stdafx.h"
#include "CppUnitTest.h"
#include "SerialTimerResponseHandlerMock.h"
#include "BaseSerialHandlerMock.h"
#include <Arduino.h>
#include "SerialStream.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudiControlUnitTest
{
	TEST_CLASS(SerialTimeResponseHandlerTest)
	{
	public:

		TEST_METHOD(BufferOverflowTest)
		{
			timeOffset = 0;
			char data1[] = "some arg"; // length = 8

			SerialStream serial;
			SerialTimerResponseHandlerMock timerResponseHandler(&serial);

			timerResponseHandler.Loop();
			wchar_t message[128];

			int maxIterations = SERIAL_RX_BUFFER_SIZE / strlen(data1) + 1;

			for (int i = 0; i < maxIterations; i++) {
				serial.AddRXBuffer(data1);
				timerResponseHandler.Loop();
				if (timerResponseHandler.receivedLength > 0) {

					if (!timerResponseHandler.isResponseOverFlow) {
						swprintf(message, 128, L"Command detected before owerflow %d \"%S\"", i, timerResponseHandler.receivedCommand);
						Assert::Fail(message);
					}
					else if (i != maxIterations - 2) {
						swprintf(message, 128, L"Command owerflow happened on wrong index %d \"%S\"", (i + 1), timerResponseHandler.receivedCommand);
						Assert::Fail(message);
					}
					return;
				}
			}

			swprintf(message, 128, L"Command owerflow has not been detected\"%S\"", timerResponseHandler.receivedCommand);
			Assert::Fail(message);
		}

		TEST_METHOD(ResponseTimerTest)
		{
			char data1[] = "1"; // length = 8

			timeOffset = 0;
			SerialStream serial;
			SerialTimerResponseHandlerMock timerResponseHandler(&serial);

			Timer::Loop();
			timerResponseHandler.Loop();
			wchar_t message[128];

			int maxIterations = 2;

			for (int i = 0; i < maxIterations; i++) {
				serial.AddRXBuffer(data1);

				timerResponseHandler.Loop();
				if (timerResponseHandler.isResponseReceived) {
					if (timerResponseHandler.ResponseByteTimeOut() < micros()) {
						swprintf(message, 128, L"Command detected befor timer runs out %d \"%S\" %lu < %lu", i, timerResponseHandler.receivedCommand, timerResponseHandler.ResponseByteTimeOut(), micros());
						Assert::Fail(message);
					}

					if (timerResponseHandler.isResponseOverFlow) {
						swprintf(message, 128, L"Command detected as owerflow %d \"%S\"", i, timerResponseHandler.receivedCommand);
						Assert::Fail(message);
					}
					if (timerResponseHandler.isResponseTimeOut) {
						swprintf(message, 128, L"Command detected as time out %d \"%S\"", i, timerResponseHandler.receivedCommand);
						Assert::Fail(message);
					}
					return;
				}
				timeOffset += timerResponseHandler.ResponseByteTimeOut();
				Timer::Loop();
			}

			swprintf(message, 128, L"Command data has not been detected \"%S\"", timerResponseHandler.receivedCommand);
			Assert::Fail(message);
		}

	};
}