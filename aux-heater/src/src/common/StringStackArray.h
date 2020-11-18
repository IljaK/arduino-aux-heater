#include "StackArray.h"

class StringStackArray: public StackArray<char *>
{
public:
    StringStackArray(const uint8_t maxSize):StackArray(maxSize) {}

    virtual ~StringStackArray()
    {
        for(uint8_t i = 0; i < maxSize; i++) {
            if (arr[i] == NULL) break;
            free(arr[i]);
            arr[i] = NULL;
        }
    }

    bool IsElementEqual(char * item1, char * item2) override
    {
        if (item1 != NULL && item2 != NULL) {
            return strcmp(item1, item2) == 0;
        }
        return StackArray::IsElementEqual(item1, item2);
    }

    char *AppendCopy(char *item)
    {
        if (item == NULL) return NULL;
        if (item[0] == 0) return NULL;
        if (IsFull()) return NULL;
        if (Contains(item)) return NULL;

        char *pCopy = (char *)malloc(strlen(item) + 1);
        strcpy(pCopy, item);
        if (!Append(pCopy)) {
            free(pCopy);
            return NULL;
        }

        return pCopy;
    }
};