#include "SerialCharResponseHandler.h"

SerialCharResponseHandler::SerialCharResponseHandler(const char *separator, Stream * serial):BaseSerialHandler(serial)
{
	this->separator = new char[strlen(separator) + 1];
	strcpy(this->separator, separator);
	buffer[0] = 0;
	separatorLength = strlen(separator);
}

SerialCharResponseHandler::~SerialCharResponseHandler()
{
    delete(this->separator);
}

void SerialCharResponseHandler::Loop()
{
	if (serial) {
		int available = serial->available();

		if (available > 0) {
			uint8_t symbol = 0;
			for (int i = 0; i < available; i++) {

				serial->readBytes(&symbol, 1);

				if (LoadSymbolFromBuffer(symbol)) return;
			}
		}
	}

	BaseSerialHandler::Loop();
}

bool SerialCharResponseHandler::LoadSymbolFromBuffer(uint8_t symbol)
{
	// sepparator symbol match
	if (separator[separatorMatchedLength] == symbol) {
		separatorMatchedLength++;

		if (separatorMatchedLength >= separatorLength) {
			separatorMatchedLength = 0;
			ResponseDetectedInternal(false, false);
			return true;
		}
		return false;
	}

	// Append skipped matched sepparator characters
	if (separatorMatchedLength > 0) {

		// Append characters to buffer + recheck on separator match
		for (int i = 0; i < (int)separatorMatchedLength; i++) {

			if (!AppendSymbolToBuffer(separator[i])) {
				ResponseDetectedInternal(false, true);
				return true;
			}

			// TODO: check if las symbol of buff + remain separator chars + new symbol match separator
			if (IsSeparatorRemainMatch(separatorMatchedLength - i - 1, symbol)) {
				separatorMatchedLength = separatorMatchedLength - i;
				return false;
			}
		}
	}
	if (!AppendSymbolToBuffer(symbol)) {
		ResponseDetectedInternal(false, true);
		return true;
	}
	return false;
}


bool SerialCharResponseHandler::IsSeparatorRemainMatch(int remainSeparatorLength, uint8_t symbol)
{
	char temp[sizeof(separator) + 1];
	temp[0] = buffer[bufferLength - 1];
	for (int i = 0; i < remainSeparatorLength; i++) {
		temp[i + 1] = separator[i];
	}
	temp[remainSeparatorLength] = symbol;
	temp[remainSeparatorLength + 1] = 0;

	return strncmp(temp, separator, remainSeparatorLength + 1) == 0;
}

bool SerialCharResponseHandler::AppendSymbolToBuffer(uint8_t symbol)
{
	if (bufferLength + 1 >= SERIAL_CHAR_BUFFER_SIZE) {
		return false;
	}

	buffer[bufferLength] = symbol;
	bufferLength++;
	buffer[bufferLength] = 0;

	return true;
}

void SerialCharResponseHandler::ResponseDetectedInternal(bool IsTimeOut, bool isOverFlow) {
	BaseSerialHandler::ResponseDetectedInternal(IsTimeOut, isOverFlow);
	ResetBuffer();
}

bool SerialCharResponseHandler::IsBusy()
{
	return BaseSerialHandler::IsBusy();
}

void SerialCharResponseHandler::FlushData()
{
	ResetBuffer();
	BaseSerialHandler::FlushData();
}

void SerialCharResponseHandler::ResetBuffer() {
	separatorMatchedLength = 0;
	buffer[0] = 0;
	bufferLength = 0;
}
