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
constexpr uint32_t MICROS_IN_SECOND = 1000000ul;

struct tmZone: public tm
{
    int8_t quaterZone = 0;

    uint32_t ZoneInSeconds() {
        return (uint32_t)quaterZone * (uint32_t)QUATER_HOUR_SECONDS;
    }

    uint16_t ZoneInMinutes() {
        return (uint16_t)quaterZone * (uint32_t)QUATER_HOUR_MINUTES;
    }
};

class TimeManager {
private:
    static uint32_t syncMicroTS;
    static time_t launchTimeStamp;
    static time_t currentTimeStamp;
    static int8_t quaterZone;

    static uint32_t getSecondsSinceUpdate();

public:
    static void LateLoop();
    static time_t GetLocalTime();
    static time_t GetUtcTime();
    static uint32_t GetOnlineSeconds();
    static time_t GetTimeSeconds(tm *tmStruct);

    static void TimeLocalStruct(char *localTime, tmZone *tmStruct);
    static void TimeUTCStruct(char *utcTime, tmZone *tmStruct);
    static void TimeStruct(tm *tmStruct, char **dateArray, char **timeArray);

    static void UpdateSystemTime(tmZone *tmStruct);
};

/*
extern unsigned long prevMicrosSeconds;
extern uint32_t launchTime;

extern void timeLocalStruct(char *localTime, tmZone *tmStruct);
extern void timeUTCStruct(char *utcTime, tmZone *tmStruct);
extern void timeStruct(tm *tmStruct, char **dateArray, char **timeArray);
extern void setSystemTime(tmZone *tmStruct);
extern void updateTime();
extern time_t getTimeSeconds(tm *tmStruct);

extern time_t getOnlineDuration();
*/