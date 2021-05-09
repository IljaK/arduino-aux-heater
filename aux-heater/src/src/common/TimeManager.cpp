#include "TimeManager.h"

uint32_t TimeManager::syncMicroTS = 0;
time_t TimeManager::launchTimeStamp = 0;
time_t TimeManager::currentTimeStamp = 0;
int8_t TimeManager::quaterZone = 0;

void TimeManager::LateLoop()
{
 	uint32_t seconds = getSecondsSinceUpdate();
	if (seconds > 0) {
        currentTimeStamp += seconds;
        syncMicroTS += (seconds * MICROS_IN_SECOND);

        /*
        DebugHandler::outWrite("Time: ");
        time_t localTime = GetLocalTime();
        DebugHandler::outWrite(ctime(&localTime));
        DebugHandler::outWriteEnd();
        DebugHandler::outWrite("Online: ");
        DebugHandler::outWrite(GetOnlineSeconds());
        DebugHandler::outWriteEnd();
        */

	}
}

time_t TimeManager::GetTimeSeconds(tm *tmStruct)
{
#if defined(ESP32) || defined(ARDUINO_ARCH_SAMD)
	return mktime(tmStruct);
#else
	return mk_gmtime(tmStruct);
#endif
}

time_t TimeManager::GetLocalTime()
{
    return GetUtcTime() + (time_t)quaterZone;
}

time_t TimeManager::GetUtcTime()
{
    return currentTimeStamp + getSecondsSinceUpdate();
}

uint32_t TimeManager::GetOnlineSeconds()
{
    return GetUtcTime() - launchTimeStamp;
}

void TimeManager::TimeLocalStruct(char *localTime, tmZone *tmStruct)
{
    // localTime format: "20/08/25,21:08:38+12"
    localTime = ShiftQuotations(localTime);

	// Split string to date & time
	char *dataArray[2];
	SplitString(localTime, ',', dataArray, 2, false);

	// dataArray[0] - date 20/08/25
	// dataArray[1] - time 21:08:38+12

	char *dateArray[3];
	SplitString(dataArray[0], '/', dateArray, 3, false);
	char *timeArray[3];
	SplitString(dataArray[1], ':', timeArray, 3, false);

	char *zPointer = strchr(timeArray[2], '+');
    if (!zPointer) zPointer = strchr(timeArray[2], '-');

    if (zPointer) {
        tmStruct->quaterZone = atoi(zPointer);
        zPointer[0] = 0;
    }

    TimeStruct(tmStruct, dateArray, timeArray);
}
void TimeManager::TimeUTCStruct(char *utcTime, tmZone *tmStruct)
{
    // utcTime format: "2020,8,31,5,46,2,"+12",1
    utcTime = ShiftQuotations(utcTime);
    char *dateArray[8];
    SplitString(utcTime, ',', dateArray, 8, false);
    char **timeArray = dateArray + 3;

    timeArray[3] = ShiftQuotations(timeArray[3]);

    tmStruct->quaterZone = atoi(timeArray[3]);
	tmStruct->tm_isdst = atoi(timeArray[4]);

    TimeStruct(tmStruct, dateArray, timeArray);
}
void TimeManager::TimeStruct(tm *tmStruct, char **dateArray, char **timeArray)
{
    // dateArray[3] format: year,month,day
    // timeArray[3] format: hour, min, second
    uint16_t year = atoi(dateArray[0]);
	if (year < 100) year += 2000u;

	tmStruct->tm_year = year - 1900u;
	tmStruct->tm_mon = atoi(dateArray[1]) - 1;
	tmStruct->tm_mday = atoi(dateArray[2]);
	tmStruct->tm_hour = atoi(timeArray[0]);
	tmStruct->tm_min = atoi(timeArray[1]);
	tmStruct->tm_sec = atoi(timeArray[2]);
}


uint32_t TimeManager::getSecondsSinceUpdate()
{
    uint32_t microsTS = micros();
	return (microsTS - syncMicroTS) / MICROS_IN_SECOND;
}

void TimeManager::UpdateSystemTime(tmZone *tmStruct)
{
    syncMicroTS = micros();
    uint32_t prevTS = currentTimeStamp;
    currentTimeStamp = GetTimeSeconds(tmStruct) - tmStruct->ZoneInSeconds();

    launchTimeStamp += difftime(currentTimeStamp, prevTS);
}
