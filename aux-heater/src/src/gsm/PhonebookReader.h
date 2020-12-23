#include <Arduino.h>
#include "PhoneEntryArray.h"

#define MAX_PHONES_STORE 3u

class PhonebookReader
{
private:
    uint8_t minSimIndex = 0;
    uint8_t maxSimIndex = 0;
    PhoneEntryArray entryArray = PhoneEntryArray(MAX_PHONES_STORE);

public:
    PhonebookReader();
    void HandleSimEntries(uint8_t min, uint8_t max);
    void HandleEntriy(char * phone, char * name);

    uint8_t GetMinSimIndex();
    uint8_t GetMaxSimIndex();

    char *GetPrimaryPhone();
};