#pragma once
#include "StackArray.h"

struct ByteArray {
    uint8_t * array;
    uint8_t length;
};

class ByteStackArray: public StackArray<ByteArray *>
{
private:
    uint8_t maxItemLength = 0;

    uint8_t AppendToItem(ByteArray *item , uint8_t * data, uint8_t length) {

        uint8_t append = maxItemLength - item->length;
        if (append > length) append = length;

        uint8_t * array = (uint8_t *)realloc(item->array, item->length + append);
        if (array == NULL) {
            // Could not reallocate memory!
            return 0;
        }

        memcpy(array + item->length, data, append);

        item->array = array;
        item->length += append;
        // Append available here
        return length - append;

    }

    uint8_t AppendInternal(uint8_t *data, uint8_t length) {
        if (length == 0) return 0;
        if (data == NULL) return 0;

        if (Size() == 0) {
            arr[0] = (ByteArray *)malloc(sizeof(ByteArray));
            return AppendToItem(arr[0], data, length);
        }

        ByteArray *lastItem = arr[Size() - 1];

        if (lastItem->length < maxItemLength) {
            return AppendToItem(lastItem, data, length);
        } 
        
        if (Size() == MaxSize()) {
            // Last element full
            return 0;
        }

        ByteArray *newItem = (ByteArray *)malloc(sizeof(ByteArray));
        arr[Size()] = newItem;
        return AppendToItem(newItem, data, length);
    }

public:
    ByteStackArray(const uint8_t maxSize, uint8_t maxItemLength):StackArray(maxSize) {
        this->maxItemLength = maxItemLength;
    }

    ~ByteStackArray()
    {
        for(uint8_t i = 0; i < maxSize; i++) {
            if (arr[i] == NULL) break;
            free(arr[i]->array);
            arr[i]->array = NULL;
        }
    }

    bool IsElementEqual(ByteArray * item1, ByteArray * item2) override {
        return false;
    }

    bool Append(ByteArray *item) override {
        // TODO:
        return false;
    }

    uint8_t Append(const uint8_t *item, uint8_t length) {
        uint8_t remain = length;
        while (remain > 0) {
            uint8_t added = AppendInternal((uint8_t *)item + (length - remain), remain);
            if (added == 0) {
                return (length - remain);
            }
            remain -= added;
        }
        return (length - remain);
    }

    bool HasReadyPacked() {
        if (Size() > 0) {
            return arr[0]->length == maxItemLength;
        }
        return false;
    }
};