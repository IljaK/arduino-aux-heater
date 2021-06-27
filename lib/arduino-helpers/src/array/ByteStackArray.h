#pragma once
#include "../common/Util.h"
#include "StackArray.h"

struct ByteArray {
    size_t length;
    uint8_t * array;
};

class ByteStackArray: public StackArray<ByteArray *>
{
private:
    size_t AppendToItem(ByteArray *item , uint8_t * data, size_t length) {

        size_t append = maxItemLength - item->length;
        if (append > length) append = length;

        //Serial.print("AppendToItem: ");
        //Serial.println(append);

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

    ByteArray * InsertNewItem(size_t index) {
        arr[index] = (ByteArray *)malloc(sizeof(ByteArray));
        arr[index]->length = 0;
        arr[index]->array = NULL;
        size++;
        return arr[index];
    }

    size_t AppendInternal(uint8_t *data, size_t length) {
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
protected:

public:
    void FreeItem(ByteArray * item) override {
        if (item != NULL) {
            if (item->array != NULL) {
                free(item->array);
            }
            free(item);
        }
    }
    const size_t maxItemLength;

    ByteStackArray(const size_t maxSize, const size_t maxItemLength):
        StackArray(maxSize),
        maxItemLength(maxItemLength) {
    }

    bool IsElementEqual(ByteArray * item1, ByteArray * item2) override {
        return false;
    }

    bool Append(ByteArray *item) override {
        // TODO:
        return false;
    }

    size_t Append(const uint8_t *item, size_t length) {
        size_t remain = length;
        while (remain > 0) {
            size_t added = AppendInternal((uint8_t *)item + (length - remain), remain);
            //Serial.print("Added:");
            //Serial.println(added);
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