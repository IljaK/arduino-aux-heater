#pragma once
#include <Stream.h>

class SerialStream: public Stream
{
public:
	void SetRXBuffer(uint8_t *byteArray, rx_buffer_index_t length);
	void AddRXBuffer(uint8_t *byteArray, rx_buffer_index_t length);

	void SetRXBuffer(char *byteArray, rx_buffer_index_t length);
	void SetRXBuffer(char *byteArray);

	void AddRXBuffer(char *byteArray, rx_buffer_index_t length);
	void AddRXBuffer(char *byteArray);
};