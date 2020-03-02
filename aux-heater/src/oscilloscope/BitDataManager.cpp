#include "BitDataManager.h"
#include <math.h>

BitDataManager::BitDataManager()
{
	FlushBitTable();
}

BitDataManager::~BitDataManager()
{
}

bool BitDataManager::AppendBitDta(uint8_t pinValue, bool newBitRow, unsigned long timeStamp)
{
	// timeout for single bit size
	//timeoutTimer.Start(32768);

	uint8_t tableIndex = ActualTableIndex();

	if (newBitRow) {
		if (bitTable[tableIndex].Length() != 0) {
			tableIndex++;
		}
		bitTable[tableIndex].bitRowBeginTS = timeStamp;
	}
	if (tableIndex >= BIT_TABLE_LENGTH) return false;

	BitRow *bitArray = &bitTable[tableIndex];

	// outPrintf("AppendBitDta [%d] [%d]", tableIndex, bitArray->size);
	// TODO: Make logic for owerflowing to next row

	if (bitArray->Length() >= MAX_BITS_AMOUNT) {
		bitArray->IsOwerFlow(true);
		if (tableIndex + 1 >= BIT_TABLE_LENGTH) {
			return false;
		}

		bitArray->bitData[bitArray->Length() - 1].Duration((uint16_t)(timeStamp - bitArray->bitRowBeginTS - BitArrayDuration(bitArray)));

		tableIndex++;
		bitArray = &bitTable[tableIndex];
		bitArray->bitRowBeginTS = timeStamp;
	}

	uint8_t actualBitIndex = bitArray->Length();
	bitArray->bitData[actualBitIndex].IsPositive(pinValue == HIGH);
	bitArray->bitData[actualBitIndex].Duration(0);

	if (actualBitIndex != 0) {
		// Set previous bit length
		bitArray->bitData[actualBitIndex - 1].Duration((uint16_t)(timeStamp - bitArray->bitRowBeginTS - BitArrayDuration(bitArray)));
	}

	bitArray->Length(actualBitIndex + 1);
	
	return true;
}

unsigned long BitDataManager::BitArrayDuration(BitRow *bitArray)
{
	unsigned long duration = 0;
	for (uint8_t i = 0; i < bitArray->Length(); i++) {
		duration += bitArray->bitData[i].Duration();
	}
	return duration;
}

BitRow *BitDataManager::BitTableRow(uint8_t index)
{
	if (index <= ActualTableIndex()) {
		return &bitTable[index];
	}
	return NULL;
}

unsigned long BitDataManager::MaxBitDuration()
{
	return (unsigned long)pow(2, (sizeof(bitTable->bitData[0].Duration()) * 8) - 1);
}

void BitDataManager::FlushBitTable()
{
	for (uint8_t i = 0; i < BIT_TABLE_LENGTH; i++) {
		bitTable[i].Length(0);
		bitTable[i].IsOwerFlow(false);
	}
}

uint8_t BitDataManager::ActualTableIndex()
{
	uint8_t tableSize = TableSize();
	if (tableSize != 0) tableSize--;
	return tableSize;
}
uint8_t BitDataManager::TableSize()
{
	for (uint8_t i = 0; i <= BIT_TABLE_LENGTH; i++) {
		if (bitTable[i].Length() == 0 || i == BIT_TABLE_LENGTH) {
			return (uint8_t)(i);
		}
	}
	return 0;
}
