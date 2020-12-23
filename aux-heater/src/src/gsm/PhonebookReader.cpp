#include "PhonebookReader.h"
#include "GSMSerialHandler.h"

PhonebookReader::PhonebookReader()
{

}

void PhonebookReader::HandleSimEntries(uint8_t min, uint8_t max)
{
    minSimIndex = min;
    maxSimIndex = max;
    entryArray.Clear();
}
void PhonebookReader::HandleEntriy(char * phone, char * name)
{
    char *auxChar = strstr(name, GSM_AUX_PHONE_POSTFIX);
    if (auxChar == NULL) return;

    auxChar += strlen(GSM_AUX_PHONE_POSTFIX);

    entryArray.Append(phone, atoi(auxChar));
}

uint8_t PhonebookReader::GetMinSimIndex()
{ 
    return minSimIndex; 
}
uint8_t PhonebookReader::GetMaxSimIndex() 
{ 
    return maxSimIndex; 
}

char *PhonebookReader::GetPrimaryPhone()
{
    PhoneUserEntry * item = entryArray.Peek();
    if (item == NULL) return NULL;
    return item->phone;
}
