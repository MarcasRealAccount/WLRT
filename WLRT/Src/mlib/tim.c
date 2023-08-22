#include "tim.h"
#include "build.h"

#include <time.h>

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>
	#include <powerbase.h>
	#include <intrin.h>

	#pragma comment(lib, "PowrProf.lib")

typedef struct _PROCESSOR_POWER_INFORMATION
{
	ULONG Number;
	ULONG MaxMhz;
	ULONG CurrentMhz;
	ULONG MhzLimit;
	ULONG MaxIdleState;
	ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;
#else
#endif

static bool     s_HRSupported = false;
static uint64_t s_HRF         = 1;
static double   s_HRFactor    = 1;
#if BUILD_IS_SYSTEM_WINDOWS
static uint64_t s_QPF       = 1;
static double   s_QPCFactor = 1;
#else
#endif

bool mtime_init()
{
#if BUILD_IS_SYSTEM_WINDOWS
	do {
		int res[4];
		__cpuid(res, 0x80000001);
		if (!(res[2] & 0x00800000))
			break;
		__cpuid(res, 0x000000001);
		if (!(res[3] & 0x00000010))
			break;
		__cpuid(res, 0x80000007);
		if (!(res[3] & 0x00000100))
			break;
		s_HRSupported = true;

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		PROCESSOR_POWER_INFORMATION* infos = (PROCESSOR_POWER_INFORMATION*) mmalloc(sysInfo.dwNumberOfProcessors * sizeof(PROCESSOR_POWER_INFORMATION));
		CallNtPowerInformation(ProcessorInformation, NULL, 0, infos, sysInfo.dwNumberOfProcessors * sizeof(PROCESSOR_POWER_INFORMATION));
		s_HRF      = infos->MaxMhz * 1000000ULL;
		s_HRFactor = 1e9 / s_HRF;
		mfree(infos);
	}
	while (false);
	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	s_QPF       = qpf.QuadPart;
	s_QPCFactor = 1e9 / s_QPF;
#else
#endif
	return true;
}

void mtime_deinit()
{
}

mdate_t mdate_utc_now()
{
#if BUILD_IS_SYSTEM_WINDOWS
	SYSTEMTIME time;
	GetSystemTime(&time);
	mdate_t date = {
		.year        = (uint16_t) time.wYear,
		.month       = (uint8_t) time.wMonth,
		.dayOfWeek   = (uint8_t) time.wDay,
		.day         = (uint8_t) time.wDay,
		.hour        = (uint8_t) time.wHour,
		.minute      = (uint8_t) time.wMinute,
		.second      = (uint8_t) time.wSecond,
		.millisecond = (uint16_t) time.wMilliseconds,
		.timezone    = 0
	};
	return date;
#else
#endif
}

mdate_t mdate_local_now()
{
#if BUILD_IS_SYSTEM_WINDOWS
	SYSTEMTIME time;
	GetLocalTime(&time);
	mdate_t date = {
		.year        = (uint16_t) time.wYear,
		.month       = (uint8_t) time.wMonth,
		.dayOfWeek   = (uint8_t) time.wDay,
		.day         = (uint8_t) time.wDay,
		.hour        = (uint8_t) time.wHour,
		.minute      = (uint8_t) time.wMinute,
		.second      = (uint8_t) time.wSecond,
		.millisecond = (uint16_t) time.wMilliseconds,
		.timezone    = mdate_timezone()
	};
	return date;
#else
#endif
}

int8_t mdate_timezone()
{
#if BUILD_IS_SYSTEM_WINDOWS
	TIME_ZONE_INFORMATION timeZone;
	GetTimeZoneInformation(&timeZone);
	return (int8_t) (-timeZone.DaylightBias / 60);
#else
#endif
}

static int16_t s_NonLeapYearDayOffsets[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static int16_t s_LeapYearDayOffsets[12]    = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

int64_t mdate_to_unix(mdate_t date)
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

mdate_t mdate_from_unix(int64_t unix)
{
	int64_t milliseconds = unix % 1000;
	unix                /= 1000;
	struct tm tdate;
	memset(&tdate, 0, sizeof(tdate));
#if BUILD_IS_SYSTEM_WINDOWS
	gmtime_s(&tdate, &((time_t) unix));
#else
#endif
	mdate_t date = {
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

uint64_t mtime_steady_freq()
{
#if BUILD_IS_SYSTEM_WINDOWS
	return s_QPF;
#else
#endif
}

uint64_t mtime_high_res_freq()
{
	return s_HRSupported ? s_HRF : mtime_steady_freq();
}

double mtime_steady_factor()
{
#if BUILD_IS_SYSTEM_WINDOWS
	return s_QPCFactor;
#else
#endif
}

double mtime_high_res_factor()
{
	return s_HRSupported ? s_HRFactor : mtime_steady_factor();
}

uint64_t mtime_steady()
{
#if BUILD_IS_SYSTEM_WINDOWS
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (uint64_t) counter.QuadPart;
#else
#endif
}

uint64_t mtime_high_res()
{
	if (!s_HRSupported)
		return mtime_steady();

#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) __rdtsc();
#else
#endif
}

mstringview_t mmonth_to_string(uint8_t month)
{
	switch (month)
	{
	case 1: return mstringviewcstr("January");
	case 2: return mstringviewcstr("February");
	case 3: return mstringviewcstr("March");
	case 4: return mstringviewcstr("April");
	case 5: return mstringviewcstr("May");
	case 6: return mstringviewcstr("June");
	case 7: return mstringviewcstr("July");
	case 8: return mstringviewcstr("August");
	case 9: return mstringviewcstr("September");
	case 10: return mstringviewcstr("October");
	case 11: return mstringviewcstr("November");
	case 12: return mstringviewcstr("December");
	default: return mstringviewcstr(NULL);
	}
}

mstringview_t mmonth_to_short_string(uint8_t month)
{
	switch (month)
	{
	case 1: return mstringviewcstr("Jan");
	case 2: return mstringviewcstr("Feb");
	case 3: return mstringviewcstr("Mar");
	case 4: return mstringviewcstr("Apr");
	case 5: return mstringviewcstr("May");
	case 6: return mstringviewcstr("Jun");
	case 7: return mstringviewcstr("Jul");
	case 8: return mstringviewcstr("Aug");
	case 9: return mstringviewcstr("Sep");
	case 10: return mstringviewcstr("Oct");
	case 11: return mstringviewcstr("Nov");
	case 12: return mstringviewcstr("Dec");
	default: return mstringviewcstr(NULL);
	}
}

mstringview_t mday_of_week_to_string(uint8_t dayOfWeek)
{
	switch (dayOfWeek)
	{
	case 1: return mstringviewcstr("Monday");
	case 2: return mstringviewcstr("Tuesday");
	case 3: return mstringviewcstr("Wednesday");
	case 4: return mstringviewcstr("Thursday");
	case 5: return mstringviewcstr("Friday");
	case 6: return mstringviewcstr("Saturday");
	case 7: return mstringviewcstr("Sunday");
	default: return mstringviewcstr(NULL);
	}
}

mstringview_t mday_of_week_to_short_string(uint8_t dayOfWeek)
{
	switch (dayOfWeek)
	{
	case 1: return mstringviewcstr("Mon");
	case 2: return mstringviewcstr("Tue");
	case 3: return mstringviewcstr("Wed");
	case 4: return mstringviewcstr("Thu");
	case 5: return mstringviewcstr("Fri");
	case 6: return mstringviewcstr("Sat");
	case 7: return mstringviewcstr("Sun");
	default: return mstringviewcstr(NULL);
	}
}