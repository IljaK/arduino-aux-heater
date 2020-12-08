#include "TimeUtil.h"

unsigned long prevMicrosSeconds = 0;
uint32_t startTime = 0;

// localTime format: "20/08/25,21:08:38+12"
void timeLocalStruct(char *localTime, tmZone *tmStruct) {
	// Cut quotations
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

    timeStruct(tmStruct, dateArray, timeArray);
}

// utcTime format: "2020,8,31,5,46,2,"+12",1
void timeUTCStruct(char *utcTime, tmZone *tmStruct)
{
    utcTime = ShiftQuotations(utcTime);
    char *dateArray[8];
    SplitString(utcTime, ',', dateArray, 8, false);
    char **timeArray = dateArray + 3;

    timeArray[3] = ShiftQuotations(timeArray[3]);

    tmStruct->quaterZone = atoi(timeArray[3]);
	tmStruct->tm_isdst = atoi(timeArray[4]);

    timeStruct(tmStruct, dateArray, timeArray);
}

// dateArray[3] format: year,month,day
// timeArray[3] format: hour, min, second
void timeStruct(tm *tmStruct, char **dateArray, char **timeArray)
{
	uint16_t year = atoi(dateArray[0]);
	if (year < 100) year += 2000u;

	tmStruct->tm_year = year - 1900u;
	tmStruct->tm_mon = atoi(dateArray[1]) - 1;
	tmStruct->tm_mday = atoi(dateArray[2]);
	tmStruct->tm_hour = atoi(timeArray[0]);
	tmStruct->tm_min = atoi(timeArray[1]);
	tmStruct->tm_sec = atoi(timeArray[2]);
}

void setSystemTime(tmZone * tmStruct)
{
	prevMicrosSeconds = micros();
	tmStruct->tm_isdst = 0;

#if defined(ESP32)
	timeval tmVal;
	tmVal.tv_usec = 0;
	tmVal.tv_sec = mktime(tmStruct);
	timezone tZone;
	tZone.tz_dsttime = 0;
	tZone.tz_minuteswest = tmStruct->ZoneInMinutes();

	settimeofday(&tmVal, &tZone);
#elif defined(ARDUINO_ARCH_SAMD)

    // TODO:

#else
    set_zone(tmStruct->ZoneInSeconds());
	set_system_time(mktime(tmStruct));

#endif
}

void updateTime()
{
	if (prevMicrosSeconds == 0) return;
#if !defined(ESP32) && !defined(ARDUINO_ARCH_SAMD)
	uint32_t microsTS = micros();
	uint32_t seconds = (microsTS - prevMicrosSeconds) / 1000000ul;
	if (seconds > 0) {

        set_system_time(time(NULL) + (time_t )seconds);

        //for (unsigned long i = 0; i < seconds; i++) {
        //    system_tick();
        //}
		prevMicrosSeconds += (seconds * 1000000ul);
	}
#endif
}
time_t getTimeSeconds(tm *tmStruct)
{
#if defined(ESP32) || defined(ARDUINO_ARCH_SAMD)
	return mktime(tmStruct);
#else
	return mk_gmtime(tmStruct);
#endif
}