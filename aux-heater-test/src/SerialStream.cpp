#include "SerialStream.h"

void SerialStream::SetRXBuffer(uint8_t *byteArray, rx_buffer_index_t length)
{
	if (length > SERIAL_RX_BUFFER_SIZE) length = SERIAL_RX_BUFFER_SIZE;

	for (rx_buffer_index_t i = 0; i < length; i++) {
		buffer[i] = byteArray[i];
	}
	this->length = length;
}


void SerialStream::AddRXBuffer(uint8_t *byteArray, rx_buffer_index_t length)
{
	if (SERIAL_RX_BUFFER_SIZE - this->length < length) {
		length = SERIAL_RX_BUFFER_SIZE - this->length;
	}

	if (length == 0) return;

	for (rx_buffer_index_t i = 0; i < length; i++) {
		buffer[i + this->length] = byteArray[i];
	}
	this->length += length;
}

void SerialStream::SetRXBuffer(char *str, rx_buffer_index_t length)
{
	SetRXBuffer((uint8_t *)str, length);
}
void SerialStream::SetRXBuffer(char *str)
{
	SetRXBuffer((uint8_t *)str, (rx_buffer_index_t)strlen(str));
}

void SerialStream::AddRXBuffer(char *str, rx_buffer_index_t length)
{
	AddRXBuffer((uint8_t *)str, length);
}
void SerialStream::AddRXBuffer(char *str)
{
	AddRXBuffer((uint8_t *)str, (rx_buffer_index_t)strlen(str));
}
