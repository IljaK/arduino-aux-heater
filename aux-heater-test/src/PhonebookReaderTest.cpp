#include <gtest/gtest.h>
#include <Arduino.h>
#include "gsm/PhonebookReader.h"


TEST(PhonebookReaderTest, PhonebookReaderTest)
{
    PhonebookReader phonebookReader = PhonebookReader();
    phonebookReader.HandleSimEntries(1, 20);

    phonebookReader.HandleEntriy("+5", "1 aux-5");
    phonebookReader.HandleEntriy("+2", "1 aux-2");
    phonebookReader.HandleEntriy("+8", "1 aux-8");
    phonebookReader.HandleEntriy("+3", "1 aux-3");
    phonebookReader.HandleEntriy("+4", "1 aux-4");
    phonebookReader.HandleEntriy("+7", "1 aux-7");
    phonebookReader.HandleEntriy("+6", "1 aux-6");

    char *primary = phonebookReader.GetPrimaryPhone();
    
}