#include "StackArray.h"

class StringStackArray: public StackArray<char *>
{
protected:
    void FreeItem(char * item) override {
        free(item);
    }
public:
    StringStackArray(const uint8_t maxSize):StackArray(maxSize) {}

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