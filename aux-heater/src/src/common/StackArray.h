#pragma once
#include <Arduino.h>

template<typename T>
class StackArray
{
protected:
    T *arr = NULL;
    uint8_t maxSize = 0;
    uint8_t size = 0;
    virtual bool IsElementEqual(T item1, T item2)
    {
        return item1 == item2;
    }

public:
    StackArray (const uint8_t maxSize)
    {
        arr = (T *) calloc(maxSize, sizeof(T));
        this->maxSize = maxSize;
    }

    ~StackArray()
    {
        free(arr);
        arr = NULL;
    }
    virtual bool Append(T item)
    {
        if (size < maxSize) {
            arr[size] = item;
            return true;
        }
        return false;
    }
    virtual T UnshiftFirst()
    {
        T first = arr[0];
        for(uint8_t i = 0; i < maxSize; i++) {
            if (i == maxSize - 1) {
                arr[i] = NULL;
                break;
            }
            arr[i] = arr[i+1];
            if (arr[i] == NULL) break;
        }
        return first;
    }
    uint8_t MaxSize()
    {
        if (arr == NULL) return 0;
        return maxSize;
    }
    uint8_t Size()
    {
        return size;
    }
    bool IsFull()
    {
        return (arr != NULL) && Size() == maxSize;
    }

    virtual bool Contains(T item)
    {
        for(uint8_t i = 0; i < maxSize; i++) {
            if (arr[i] == NULL) return false;
            if (IsElementEqual(item, arr[i])) return true;
        }
        return false;
    }

};