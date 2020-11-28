#pragma once
#include "StackArray.h"

struct BinaryMessage {
    uint8_t length;
    uint8_t filled;
    uint8_t * data;

    bool isFilled() {
        return length == filled;
    }
};

typedef void (*BinaryMessageCallback)(BinaryMessage *);

class BinaryMessageStack: public StackArray<BinaryMessage *>
{
private:

public:
    BinaryMessageStack(const uint8_t maxSize):StackArray(maxSize) {}

    virtual ~BinaryMessageStack() {
        for(uint8_t i = 0; i < size; i++) {
            if (arr[i] == NULL) break;
            free(arr[i]->data);
            free(arr[i]);
            arr[i] = NULL;
        }
    }

    BinaryMessage * Peek() {
        if (size > 0) {
            return arr[0];
        }
        return NULL;
    }
    BinaryMessage * PeekLast() {
        if (size > 0) {
            return arr[size - 1];
        }
        return NULL;
    }

    BinaryMessage * UnshiftLast() {
        if (size > 0) {
            size--;
            return arr[size];
        }
        return NULL;
    }
};