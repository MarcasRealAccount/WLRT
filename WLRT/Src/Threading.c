#include "Threading.h"
#include "Build.h"
#include "Memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if BUILD_IS_SYSTEM_WINDOWS

	#include <Windows.h>

	#include <intrin.h>

static DWORD WLRTThreadThunk(LPVOID data)
{
	WLRTThreadData* thread = (WLRTThreadData*) data;
	return (DWORD) thread->callback(thread->userData);
}

bool WLRTThreadSetup(WLRTThreadData* thread)
{
	if (!thread || !thread->callback)
		return false;

	thread->nameLen  = 0;
	thread->nameBuf  = NULL;
	thread->native   = (void*) CreateThread(NULL, 0, &WLRTThreadThunk, thread, CREATE_SUSPENDED, NULL);
	thread->exitCode = 0;
	return thread->native != NULL;
}

void WLRTThreadCleanup(WLRTThreadData* thread)
{
	if (!thread)
		return;

	if (thread->native)
	{
		WaitForSingleObject((HANDLE) thread->native, ~0U);
		DWORD exitCode = 0;
		GetExitCodeThread((HANDLE) thread->native, &exitCode);
		thread->exitCode = (int) exitCode;
		CloseHandle((HANDLE) thread->native);
		thread->native = NULL;
	}
}

bool WLRTThreadJoin(WLRTThreadData* thread, int* exitCode)
{
	if (exitCode) *exitCode = 0;
	if (!thread || !thread->native)
		return false;

	WaitForSingleObject((HANDLE) thread->native, ~0U);
	DWORD dwExitCode = 0;
	if (!GetExitCodeThread((HANDLE) thread->native, &dwExitCode))
		return false;
	thread->exitCode = (int) dwExitCode;
	if (exitCode) *exitCode = (int) dwExitCode;
	return true;
}

void WLRTThreadStart(WLRTThreadData* thread)
{
	if (!thread || !thread->native)
		return;

	ResumeThread((HANDLE) thread->native);
}

void WLRTThreadSetName(WLRTThreadData* thread, const char* name, size_t length)
{
	if (!thread)
		return;

	if (thread->nameBuf)
		WLRTFree(thread->nameBuf, alignof(char));

	if (length == ~0ULL)
		length = strlen(name);
	thread->nameLen = length;
	thread->nameBuf = WLRTAlloc((length + 1) * sizeof(char), alignof(char));
	if (!thread->nameBuf)
	{
		thread->nameLen = 0;
		return;
	}
	memcpy(thread->nameBuf, name, thread->nameLen);
	thread->nameBuf[thread->nameLen] = '\0';

	if (thread->native)
	{
		wchar_t* buf = WLRTAlloc((thread->nameLen + 1) * sizeof(wchar_t), alignof(wchar_t));
		if (!buf)
			return;

		size_t written = mbstowcs(buf, thread->nameBuf, thread->nameLen);
		buf[written]   = L'\0';
		SetThreadDescription((HANDLE) thread->native, buf);
		WLRTFree(buf, alignof(wchar_t));
	}
}

const char* WLRTThreadGetName(WLRTThreadData* thread)
{
	if (!thread)
		return NULL;
	return thread->nameBuf ? thread->nameBuf : "Thread";
}

uint32_t WLRTAtomic32Read(volatile uint32_t* memory)
{
	return *memory;
}

uint32_t WLRTAtomic32CompareExchange(volatile uint32_t* memory, uint32_t expected, uint32_t value)
{
	return (uint32_t) InterlockedCompareExchange((volatile LONG*) memory, (LONG) value, (LONG) expected);
}

uint32_t WLRTAtomic32Exchange(volatile uint32_t* memory, uint32_t value)
{
	return (uint32_t) InterlockedExchange((volatile LONG*) memory, (LONG) value);
}

uint32_t WLRTAtomic32Add(volatile uint32_t* memory, uint32_t value)
{
	return (uint32_t) InterlockedAdd((volatile LONG*) memory, (LONG) value);
}

uint32_t WLRTAtomic32Sub(volatile uint32_t* memory, uint32_t value)
{
	return WLRTAtomic32Add(memory, ~value + 1);
}

uint64_t WLRTAtomic64Read(volatile uint64_t* memory)
{
	return *memory;
}

uint64_t WLRTAtomic64CompareExchange(volatile uint64_t* memory, uint64_t expected, uint64_t value)
{
	return (uint64_t) InterlockedCompareExchange64((volatile LONG64*) memory, (LONG64) value, (LONG64) expected);
}

uint64_t WLRTAtomic64Exchange(volatile uint64_t* memory, uint64_t value)
{
	return (uint64_t) InterlockedExchange64((volatile LONG64*) memory, (LONG64) value);
}

uint64_t WLRTAtomic64Add(volatile uint64_t* memory, uint64_t value)
{
	return (uint64_t) InterlockedAdd64((volatile LONG64*) memory, (LONG64) value);
}

uint64_t WLRTAtomic64Sub(volatile uint64_t* memory, uint64_t value)
{
	return WLRTAtomic64Add(memory, ~value + 1);
}

void WLRTAtomicWait(volatile void* memory, const void* expected, size_t expectedSize)
{
	WaitOnAddress(memory, (PVOID) expected, expectedSize, ~0U);
}

void WLRTAtomicWakeOne(void* memory)
{
	WakeByAddressSingle(memory);
}

void WLRTAtomicWakeAll(void* memory)
{
	WakeByAddressAll(memory);
}

uint32_t WLRTGetCurrentThreadId()
{
	return GetCurrentThreadId();
}

WLRTDate WLRTGetUTCDate()
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	WLRTDate date = {
		.year        = (uint16_t) time.wYear,
		.month       = (uint8_t) time.wMonth,
		.dayOfWeek   = (uint8_t) time.wDay,
		.day         = (uint8_t) time.wDay,
		.hour        = (uint8_t) time.wHour,
		.minute      = (uint8_t) time.wMinute,
		.second      = (uint8_t) time.wSecond,
		.millisecond = (uint16_t) time.wMilliseconds
	};
	return date;
}

WLRTDate WLRTGetLocalDate()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	WLRTDate date = {
		.year        = (uint16_t) time.wYear,
		.month       = (uint8_t) time.wMonth,
		.dayOfWeek   = (uint8_t) time.wDay,
		.day         = (uint8_t) time.wDay,
		.hour        = (uint8_t) time.wHour,
		.minute      = (uint8_t) time.wMinute,
		.second      = (uint8_t) time.wSecond,
		.millisecond = (uint16_t) time.wMilliseconds
	};
	return date;
}

static int16_t s_NonLeapYearDayOffsets[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static int16_t s_LeapYearDayOffsets[12]    = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

int64_t WLRTDateToUnixTime(WLRTDate date)
{
	bool isLeapYear = date.year == 1600 || date.year == 2000 || ((date.year % 4 == 0) && ((date.year % 100 != 0) || (date.year % 400 == 0)));

	uint16_t dayOfYear = (isLeapYear ? s_LeapYearDayOffsets[date.month] : s_NonLeapYearDayOffsets[date.month]) + date.day;
	int16_t  year      = date.year - 1900;

	struct tm tdate = {
		.tm_sec   = date.second,
		.tm_min   = date.minute,
		.tm_hour  = date.hour,
		.tm_mday  = date.day,
		.tm_mon   = date.month,
		.tm_year  = year,
		.tm_wday  = date.dayOfWeek,
		.tm_yday  = dayOfYear,
		.tm_isdst = 0
	};
	time_t t = mktime(&tdate);
	return ((int64_t) t) * 1000 + date.millisecond;
}

WLRTDate WLRTUnixTimeToDate(int64_t unixTime)
{
	int64_t milliseconds = unixTime % 1000;
	unixTime            /= 1000;
	struct tm tdate;
	memset(&tdate, 0, sizeof(tdate));
	gmtime_s(&tdate, &((time_t) unixTime));
	WLRTDate date = {
		.year        = (uint16_t) (tdate.tm_year + 1900),
		.month       = (uint8_t) tdate.tm_mon,
		.dayOfWeek   = (uint8_t) tdate.tm_wday,
		.day         = (uint8_t) tdate.tm_mday,
		.hour        = (uint8_t) tdate.tm_hour,
		.minute      = (uint8_t) tdate.tm_min,
		.second      = (uint8_t) tdate.tm_sec,
		.millisecond = (uint16_t) milliseconds
	};
	return date;
}

static bool          s_HRSupported = false;
static bool          s_HRQueried   = false;
static bool          s_QPFQueried  = false;
static LARGE_INTEGER s_QPF;
static uint64_t      s_QPCFactor;

uint64_t WLRTGetSteadyTime()
{
	if (!s_QPFQueried)
	{
		QueryPerformanceFrequency(&s_QPF);
		s_QPCFactor  = 1'000'000'000ULL / s_QPF.QuadPart;
		s_QPFQueried = true;
	}
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * s_QPCFactor;
}

uint64_t WLRTGetHighResTime()
{
	if (!s_HRQueried)
	{
		do {
			int res[4];
			__cpuid(res, 0x0000'0001);
			if (!(res[3] & 0x0000'0010))
				break;
			__cpuid(res, 0x8000'0007);
			if (!(res[3] & 0x000'0100))
				break;
			s_HRSupported = true;
		}
		while (false);
		s_HRQueried = true;
	}
	return s_HRSupported ? __rdtsc() : WLRTGetSteadyTime();
}

#else

	#error Threading.c, System not supported

#endif

bool WLRTMutexSetup(WLRTMutex* mutex)
{
	mutex->inUse = 0;
	return true;
}

void WLRTMutexCleanup(WLRTMutex* mutex)
{
	(void) mutex;
}

void WLRTMutexLock(WLRTMutex* mutex)
{
	uint32_t val;
	while ((val = WLRTAtomic32CompareExchange(&mutex->inUse, 0, 1)) == 1)
		WLRTAtomicWait(&mutex->inUse, &val, sizeof(uint32_t));
}

bool WLRTMutexTryLock(WLRTMutex* mutex)
{
	return WLRTAtomic32CompareExchange(&mutex->inUse, 0, 1) == 0;
}

void WLRTMutexUnlock(WLRTMutex* mutex)
{
	if (WLRTAtomic32Exchange(&mutex->inUse, 0) == 1)
		WLRTAtomicWakeOne(&mutex->inUse);
}