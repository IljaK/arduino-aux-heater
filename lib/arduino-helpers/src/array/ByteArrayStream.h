#pragma once
#include <Arduino.h>

class ByteArrayStream
{
private:
    size_t maxLength;
    size_t length;
    uint8_t * buffer;

public:
    ByteArrayStream(size_t maxLength)
    {
        this->maxLength = maxLength;
        buffer = (uint8_t *)malloc(maxLength);
        length = 0;
    }
    int Available() {
        return length;
    }
    size_t Read(uint8_t *buffer, size_t length)
    {
        if (this->length == 0) return 0;
        if (length == 0) return 0;

        size_t shifted = 0;
        size_t toCopy = length;
        if (toCopy > this->length) {
            toCopy = this->length;
        }

        size_t toIterate = this->length;
        if (toIterate < length) {
            toIterate = length;
        }

        for (size_t i = 0; i < toIterate; i++) {
            if (i < length) {
                buffer[i] = this->buffer[i];
            }
            if (toCopy + i < this->length) {
                this->buffer[i] = this->buffer[toCopy + i];
            }
        }

        this->length -= toCopy;
        return shifted;
    }
    size_t Write(uint8_t *buffer, size_t length)
    {
        size_t toWrite = length;
        if (toWrite > maxLength - this->length) {
            toWrite = maxLength - this->length;
        }
        memcpy(this->buffer + this->length, buffer, toWrite);

        this->length += toWrite;

        return toWrite;
    }
    void Flush()
    {
        length = 0;
    }
    int Peek() {
        if (length == 0) return 0;
        return buffer[0];
    }
};