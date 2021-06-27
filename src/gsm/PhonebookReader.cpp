#include "PhonebookReader.h"
#include "GSMSerialHandler.h"

PhonebookReader::PhonebookReader()
{

}

void PhonebookReader::Clear()
{
    entryArray.Clear();
}

void PhonebookReader::HandleEntriy(char * phone, char * name)
{
    char *auxChar = strstr(name, GSM_AUX_PHONE_POSTFIX);
    if (auxChar == NULL) return;

    auxChar += strlen(GSM_AUX_PHONE_POSTFIX);

    entryArray.Append(phone, atoi(auxChar));
}

char *PhonebookReader::GetPrimaryPhone()
{
    PhoneUserEntry * item = entryArray.Peek();
    if (item == NULL) return NULL;
    return item->phone;
}


bool PhonebookReader::HasNumber(char * phone)
{
    return entryArray.Contains(phone);
}
