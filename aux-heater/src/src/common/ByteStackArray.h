#pragma once
#include "Util.h"
#include "StackArray.h"

class ByteStackArray: public StackArray<ByteArray *>
{
private:
    uint8_t maxItemLength = 0;

    uint8_t AppendToItem(ByteArray *item , uint8_t * data, uint8_t length) {

        uint8_t append = maxItemLength - item->length;
        if (append > length) append = length;

        //Serial.printf("AppendToItem: %u\n", append);

        uint8_t * array = NULL;
        if (item->array == NULL) {
            array = (uint8_t *)malloc(append);
        } else {
            array = (uint8_t *)realloc(item->array, item->length + append);
        }
        if (array == NULL) {
            // Could not reallocate memory!
            return 0;
        }

        memcpy(array + item->length, data, append);

        item->array = array;
        item->length += append;
        // Append available here
        return append;

    }

    ByteArray * InsertNewItem(uint8_t index) {
        arr[index] = (ByteArray *)malloc(sizeof(ByteArray));
        arr[index]->length = 0;
        arr[index]->array = NULL;
        size++;
        return arr[index];
    }

    uint8_t AppendInternal(uint8_t *data, uint8_t length) {
        if (length == 0) return 0;
        if (data == NULL) return 0;

        if (Size() == 0) {
            return AppendToItem(InsertNewItem(0), data, length);
        }

        ByteArray *lastItem = arr[Size() - 1];

        if (lastItem->length < maxItemLength) {
            return AppendToItem(lastItem, data, length);
        } 
        
        if (Size() == MaxSize()) {
            // Last element full
            return 0;
        }
        
        return AppendToItem(InsertNewItem(Size()), data, length);
    }

public:
    ByteStackArray(const uint8_t maxSize, uint8_t maxItemLength):StackArray(maxSize) {
        this->maxItemLength = maxItemLength;
    }

    virtual ~ByteStackArray()
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
            //Serial.printf("Added: %u\n", added);
            if (added == 0) {
                break;
            }
            remain -= added;
        }
        return (length - remain);
    }

    bool HasFilledPacket() {
        if (Size() == 1) {
            return arr[0]->length == maxItemLength;
        }
        return Size() > 0;
    }
};