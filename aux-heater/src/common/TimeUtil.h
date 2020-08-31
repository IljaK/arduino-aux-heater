#pragma once
#include <Arduino.h>
#include <time.h>
#include "Util.h"

struct tmZone: public tm
{
    int8_t quaterZone = 0;

    uint32_t ZoneInSeconds() {
        return (uint32_t)quaterZone * (uint32_t)ONE_HOUR / (uint32_t)4u;
    }
};

extern void timeLocalStruct(char *localTime, tmZone *tmStruct);
extern void timeUTCStruct(char *utcTime, tmZone *tmStruct);
extern void timeStruct(tm *tmStruct, char **dateArray, char **timeArray);
extern void setSystemTime(tmZone *tmStruct);
extern void updateTime();