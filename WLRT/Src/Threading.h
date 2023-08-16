#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*WLRTThreadCallbackFn)(void* userData);
typedef struct WLRTThreadData
{
	WLRTThreadCallbackFn callback;
	void*                userData;

	size_t nameLen;
	char*  nameBuf;

	void* native;
	int   exitCode;
} WLRTThreadData;

typedef struct WLRTMutex
{
	uint32_t inUse;
} WLRTMutex;

bool        WLRTThreadSetup(WLRTThreadData* thread);
void        WLRTThreadCleanup(WLRTThreadData* thread);
bool        WLRTThreadJoin(WLRTThreadData* thread, int* exitCode);
void        WLRTThreadStart(WLRTThreadData* thread);
void        WLRTThreadSetName(WLRTThreadData* thread, const char* name, size_t length);
const char* WLRTThreadGetName(WLRTThreadData* thread);

bool WLRTMutexSetup(WLRTMutex* mutex);
void WLRTMutexCleanup(WLRTMutex* mutex);
void WLRTMutexLock(WLRTMutex* mutex);
bool WLRTMutexTryLock(WLRTMutex* mutex);
void WLRTMutexUnlock(WLRTMutex* mutex);

typedef struct WLRTDate
{
	uint16_t year;
	uint8_t  month     : 4;
	uint8_t  dayOfWeek : 4;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  minute;
	uint8_t  second;
	uint16_t millisecond;
} WLRTDate;

uint32_t WLRTGetCurrentThreadId();

WLRTDate WLRTGetUTCDate();
WLRTDate WLRTGetLocalDate();
int64_t  WLRTDateToUnixTime(WLRTDate date);
WLRTDate WLRTUnixTimeToDate(int64_t unixTime);
uint64_t WLRTGetSteadyTime();
uint64_t WLRTGetHighResTime();