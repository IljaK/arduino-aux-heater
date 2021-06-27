#include <gtest/gtest.h>
#include <Arduino.h>
#include "gsm/PhonebookReader.h"


TEST(PhonebookReaderTest, PhonebookReaderTest)
{
    PhonebookReader phonebookReader = PhonebookReader();
    //phonebookReader.HandleSimEntries(1, 20);

    phonebookReader.HandleEntriy((char *)"+5", (char *)"1 aux-5");
    phonebookReader.HandleEntriy((char *)"+2", (char *)"1 aux-2");
    phonebookReader.HandleEntriy((char *)"+8", (char *)"1 aux-8");
    phonebookReader.HandleEntriy((char *)"+3", (char *)"1 aux-3");
    phonebookReader.HandleEntriy((char *)"+4", (char *)"1 aux-4");
    phonebookReader.HandleEntriy((char *)"+7", (char *)"1 aux-7");
    phonebookReader.HandleEntriy((char *)"+6", (char *)"1 aux-6");

    char *primary = phonebookReader.GetPrimaryPhone();
    
}