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

    virtual void FreeItem(T item)
    {

    }

public:
    StackArray (const uint8_t maxSize)
    {
        arr = (T *) calloc(maxSize, sizeof(T));
        this->maxSize = maxSize;
    }

    virtual ~StackArray()
    {
        for(uint8_t i = 0; i < size; i++) {
            FreeItem(arr[i]);
            arr[i] = NULL;
        }
        size = 0;
        free(arr);
        arr = NULL;
    }

    virtual bool Append(T item)
    {
        if (size < maxSize) {
            arr[size] = item;
            size++;
            return true;
        }
        return false;
    }
    virtual T UnshiftFirst()
    {
        if (size == 0) {
            return NULL;
        }
        T first = arr[0];
        for(uint8_t i = 0; i < size; i++) {
            if (i == size - 1) {
                arr[i] = NULL;
                break;
            }
            arr[i] = arr[i+1];
            if (arr[i] == NULL) break;
        }
        size--;
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
        return size == maxSize;
    }

    virtual bool Contains(T item)
    {
        for(uint8_t i = 0; i < maxSize; i++) {
            if (arr[i] == NULL) return false;
            if (IsElementEqual(item, arr[i])) return true;
        }
        return false;
    }

    T Peek() {
        if (size > 0) {
            return arr[0];
        }
        return NULL;
    }
    T PeekLast() {
        if (size > 0) {
            return arr[size - 1];
        }
        return NULL;
    }

};