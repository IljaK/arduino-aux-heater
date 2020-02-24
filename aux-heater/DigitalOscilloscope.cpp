#include "DigitalOscilloscope.h"

DigitalOscilloscope::DigitalOscilloscope(uint8_t signalPin)
{
	this->signalPin = signalPin;
	pinMode(signalPin, INPUT);
	actualSignal = digitalRead(signalPin);
}


DigitalOscilloscope::~DigitalOscilloscope()
{
}

uint8_t DigitalOscilloscope::SignalPin()
{
	return signalPin;
}

void DigitalOscilloscope::Loop()
{
	ReadLoop(digitalRead(signalPin), micros());
}

void DigitalOscilloscope::OnTimerComplete(TimerID timerId)
{
	if (timerId == timeoutTimer) {
		unsigned long microsTS = micros();
		lastArrayCompleteTS = microsTS;
	}
}

void DigitalOscilloscope::ReadLoop(uint8_t pinValue, long microsTS)
{
	if (actualSignal != pinValue) {
		actualSignal = pinValue;
		if (!bitDataManager.AppendBitDta(pinValue, timeoutTimer == 0, microsTS))
		{
			PrintTable();
			return;
		}
		if (timeoutTimer > 0) {
			Timer::Stop(timeoutTimer);
		}
		timeoutTimer = Timer::Start(this, 32768);
	}

	if (lastArrayCompleteTS != 0 && microsTS - lastArrayCompleteTS > 500000ul) {
		PrintTable();
	}
}


void DigitalOscilloscope::PrintTable()
{
	if (timeoutTimer > 0) {
		Timer::Stop(timeoutTimer);
		timeoutTimer = 0;
	}
	lastArrayCompleteTS = 0;
	uint8_t tableSize = bitDataManager.TableSize();

	char indexLabel[8];

	outPrintf("Size: %d x %d", tableSize, MAX_BITS_AMOUNT);

	uint8_t bitRowIndex = 0;

	BitRow * bitArray = NULL;
	BitRow * prevBitArray = NULL;


	for (uint8_t i = 0; i < tableSize; i++) {
		bitArray = bitDataManager.BitTableRow(i);


		if (bitArray == NULL) break;
		if (bitArray->Length() == 0) break;

		if (prevBitArray == NULL || !prevBitArray->IsOwerFlow()) {
			bitRowIndex++;
			snprintf(indexLabel, 8, "%d) ", bitRowIndex);
			if (i != 0) {
				outWrite("\r\n");
			}
			outWrite(indexLabel);
		}

		PrintBitRow(bitArray, bitDataManager.BitTableRow(i + 1));

		if (bitArray != NULL && bitArray->IsOwerFlow()) {
			outWrite("-");
		}

		prevBitArray = bitArray;
	}

	outWrite("\r\n");
	bitDataManager.FlushBitTable();
}

void DigitalOscilloscope::PrintBitRow(BitRow *bitRow, BitRow * nextBitRow)
{
	if (bitRow == NULL) return;

	uint8_t length = bitRow->Length();
	BitData *bitInfo = bitRow->bitData;

	if (bitInfo == NULL || length == 0) return;

	char singleTimeStamp[16];

	for (uint8_t i = 0; i < length; i++) {
		if (i > 0) {
			outWrite("-");
		}

		if (i == length - 1 && nextBitRow != NULL && !bitRow->IsOwerFlow()) {
			unsigned long duration = nextBitRow->bitRowBeginTS - bitRow->bitRowBeginTS - bitDataManager.BitArrayDuration(bitRow);
			snprintf(singleTimeStamp, 16, "[%d](%lu)", bitInfo[i].Value(), duration);
		}
		else {
			snprintf(singleTimeStamp, 16, "[%d](%u)", bitInfo[i].Value(), bitInfo[i].Duration());
		}

		outWrite(singleTimeStamp);
	}
}
