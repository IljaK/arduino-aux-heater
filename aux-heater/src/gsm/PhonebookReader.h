#include <Arduino.h>
#include "PhoneEntryArray.h"

#define MAX_PHONES_STORE 3u

class PhonebookReader
{
private:
    PhoneEntryArray entryArray = PhoneEntryArray(MAX_PHONES_STORE);

public:
    PhonebookReader();
    void Clear();
    void HandleEntriy(char * phone, char * name);
    char *GetPrimaryPhone();
    bool HasNumber(char * phone);
};