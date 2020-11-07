#pragma once
#include <Arduino.h>

template<typename T>
class StackArray
{
protected:
    T *arr = NULL;
    uint8_t maxSize = 0;
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
    bool Append(T item)
    {
        for(uint8_t i = 0; i < maxSize; i++) {
            if (arr[i] == NULL) {
                arr[i] = item;
                return true;
            } else if (IsElementEqual(arr[i], item)) {
                return false;
            }
        }
        return false;
    }
    T UnshiftFirst()
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
        if (arr == NULL) return 0;
        for(uint8_t i = 0; i < maxSize; i++) {
            if (arr[i] == NULL) return i;
        }
        return maxSize;
    }
    bool IsFull()
    {
        return (arr != NULL) && Size() == maxSize;
    }

    bool Contains(T item)
    {
        for(uint8_t i = 0; i < maxSize; i++) {
            if (arr[i] == NULL) return false;
            if (IsElementEqual(item, arr[i])) return true;
        }
        return false;
    }

};