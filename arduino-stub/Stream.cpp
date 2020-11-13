#include "Stream.h"

Stream::Stream():Print()
{
}

int Stream::read()
{
	uint8_t bt = 0;
	readBytes(&bt, 1);
	return bt;
}

rx_buffer_index_t Stream::readBytes(char *buffer, rx_buffer_index_t length)
{ 
	if (this->length == 0 || length == 0) return 0;
	rx_buffer_index_t readAmount = length;
	if (readAmount > this->length)  readAmount = this->length;

	for (rx_buffer_index_t i = 0; i < this->length; i++) {
		if (i < readAmount) {
			buffer[i] = this->buffer[i];
		}
		else {
			this->buffer[i - readAmount] = this->buffer[i];
		}
	}

	this->length -= readAmount;

	return readAmount;
};

