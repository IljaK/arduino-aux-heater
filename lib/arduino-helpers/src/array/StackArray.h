#pragma once
#include <Arduino.h>

template<typename T>
class StackArray
{
protected:
    T *arr = NULL;
    const size_t maxSize;
    size_t size = 0;
    virtual bool IsElementEqual(T item1, T item2)
    {
        return item1 == item2;
    }
    virtual void FreeItem(T item) 
    {
        if (item != NULL) {
            free(item);
        }
    }

public:
    StackArray(const size_t maxSize):maxSize(maxSize)
    {
        arr = (T *) calloc(maxSize, sizeof(T));
    }

    virtual ~StackArray()
    {
        Clear();
        free(arr);
        arr = NULL;
    }

    virtual void Clear() {
        for(size_t i = 0; i < size; i++) {
            FreeItem(arr[i]);
            arr[i] = NULL;
        }
        size = 0;
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

    virtual bool Insert(T item, size_t index)
    {
        if (size < maxSize) {

            if (index >= size) {
                return Append(item);
            }

            T prevItem = NULL;
            size++;
            for (size_t i = index; i < size; i++) {
                if (prevItem == NULL) {
                    prevItem = arr[i];
                    arr[i] = item;
                } else {
                    T itm = arr[i];
                    arr[i] = prevItem;
                    prevItem = itm;
                }
            }
            return true;
        }
        return false;
    }

    virtual T UnshiftFirst()
    {
        return Unshift(0);
    }

    virtual T Unshift(size_t index)
    {
        if (size == 0) {
            return NULL;
        }
        if (index >= maxSize) {
            return NULL;
        }

        T item = arr[index];
        for(size_t i = index; i < size; i++) {
            if (i == size - 1) {
                arr[i] = NULL;
                break;
            }
            arr[i] = arr[i+1];
            if (arr[i] == NULL) break;
        }
        size--;
        return item;
    }

    size_t MaxSize()
    {
        if (arr == NULL) return 0;
        return maxSize;
    }
    size_t Size()
    {
        return size;
    }
    bool IsFull()
    {
        return size == maxSize;
    }

    virtual bool Contains(T item)
    {
        for(size_t i = 0; i < maxSize; i++) {
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

    T Peek(size_t index) {
        if (size > 0 && index < size) {
            return arr[index];
        }
        return NULL;
    }

};