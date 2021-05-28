#include "array/StackArray.h"

struct PhoneUserEntry
{
    uint8_t sortIndex = 255;
    char * phone = NULL;
};

class PhoneEntryArray: public StackArray<PhoneUserEntry *>
{
public:
    PhoneEntryArray(const uint8_t maxSize):StackArray(maxSize) {
        
    };

    virtual ~PhoneEntryArray()
    {
    }

    void Clear() {
        for(uint8_t i = 0; i < size; i++) {
            FreeItem(arr[i]);
            arr[i] = NULL;
        }
        size = 0;
    };

    void Append(char *phone, uint8_t sortIndex){
        PhoneUserEntry *newEntry = (PhoneUserEntry *) malloc(sizeof(PhoneUserEntry));

        newEntry->sortIndex = sortIndex;
        newEntry->phone = (char *)malloc(strlen(phone) + 1);
        strcpy(newEntry->phone, phone);

        if (!Append(newEntry)) {
            FreeItem(newEntry);
        }
    };

    bool Contains(char *phone)
    {
        for(uint8_t i = 0; i < size; i++) {
            if (strcmp(phone, arr[i]->phone) == 0) {
                return true;
            }
        }
        return false;
    }

protected:

    void FreeItem(PhoneUserEntry * item) override {
        if (item != NULL) {
            free(item->phone);
            free(item);
        }
    }

private:
    bool Append(PhoneUserEntry * item) override {
        
        PhoneUserEntry * shiftOutItem = NULL;
        if (size == 0) {
            arr[0] = item;
            size++;
            return true;
        }
        for (uint8_t i = 0; i < size; i++) {
            if (shiftOutItem != NULL) {
                PhoneUserEntry * temp = arr[i];
                arr[i] = shiftOutItem;
                shiftOutItem = temp;
                continue;
            }
            if (arr[i]->sortIndex > item->sortIndex) {
                shiftOutItem = arr[i];
                arr[i] = item;
            }
        }

        if (shiftOutItem == NULL) {
            if (size < maxSize) {
                arr[size] = item;
                size++;
                return true;
            }
        } else {
            if (size < maxSize) {
                arr[size] = shiftOutItem;
                size++;
            } else {
                FreeItem(shiftOutItem);
            }
            return true;
        }
        return false;
    };
};