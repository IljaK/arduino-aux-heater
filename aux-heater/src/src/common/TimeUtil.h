#pragma once
#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include "Util.h"

#ifndef ONE_HOUR
#define ONE_HOUR 3600
#endif

constexpr uint16_t QUATER_HOUR_SECONDS = (uint16_t)ONE_HOUR / (uint16_t)4u;
constexpr uint16_t QUATER_HOUR_MINUTES = (uint16_t)60 / (uint16_t)4u;

struct tmZone: public tm
{
    int8_t quaterZone = 0;

    uint32_t ZoneInSeconds() {
        return (uint32_t)quaterZone * QUATER_HOUR_SECONDS;
    }

    uint16_t ZoneInMinutes() {
        return (uint16_t)quaterZone * QUATER_HOUR_MINUTES;
    }
};

extern unsigned long prevMicrosSeconds;

extern void timeLocalStruct(char *localTime, tmZone *tmStruct);
extern void timeUTCStruct(char *utcTime, tmZone *tmStruct);
extern void timeStruct(tm *tmStruct, char **dateArray, char **timeArray);
extern void setSystemTime(tmZone *tmStruct);
extern void updateTime();
extern time_t getTimeSeconds(tm *tmStruct);

extern time_t getOnlineDuration();