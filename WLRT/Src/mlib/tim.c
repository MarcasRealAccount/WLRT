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
#elif BUILD_IS_SYSTEM_MACOSX
	#include <x86intrin.h>
	#include <cpuid.h>
	#include <sys/sysctl.h>
#else
	#include <x86intrin.h>
	#include <cpuid.h>
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
	do {
		unsigned int res[4];
		__get_cpuid(0x80000001, &res[0], &res[1], &res[2], &res[3]);
		if (!(res[2] & 0x00800000))
			break;
		__get_cpuid(0x00000001, &res[0], &res[1], &res[2], &res[3]);
		if (!(res[2] & 0x00000010))
			break;
		__get_cpuid(0x80000007, &res[0], &res[1], &res[2], &res[3]);
		if (!(res[2] & 0x00000100))
			break;
		s_HRSupported = true;

	#if BUILD_IS_SYSTEM_MACOSX
		int mib[2] = { CTL_HW, HW_CPU_FREQ };
		unsigned int freq = 0;
		size_t len = sizeof(freq);
		sysctl(mib, 2, &freq, &len, NULL, 0);
		s_HRF      = freq;
		s_HRFactor = 1e9 / s_HRF;
	#endif
	}
	while (false);
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
		.dayOfWeek   = (uint8_t) (time.wDayOfWeek == 0 ? 7 : time.wDay),
		.day         = (uint8_t) time.wDay,
		.hour        = (uint8_t) time.wHour,
		.minute      = (uint8_t) time.wMinute,
		.second      = (uint8_t) time.wSecond,
		.millisecond = (uint16_t) time.wMilliseconds,
		.timezone    = 0
	};
	return date;
#else
	time_t now = time(NULL);
	struct tm* ct = gmtime(&now); // TODO(MarcasRealAccount): gmtime(const time_t* t) might not be thread-safe.
	mdate_t date = {
		.year        = (uint16_t) (ct->tm_year + 1900),
		.month       = (uint8_t) (ct->tm_mon + 1),
		.dayOfWeek   = (uint8_t) (ct->tm_wday == 0 ? 7 : ct->tm_wday),
		.day         = (uint8_t) ct->tm_mday,
		.hour        = (uint8_t) ct->tm_hour,
		.minute      = (uint8_t) ct->tm_min,
		.second      = (uint8_t) ct->tm_sec,
		.millisecond = 0,
		.timezone    = 0
	};
	return date;
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
	time_t now = time(NULL);
	struct tm* ct = localtime(&now); // TODO(MarcasRealAccount): localtime(const time_t*) might not be thread-safe.
	mdate_t date = {
		.year        = (uint16_t) (ct->tm_year + 1900),
		.month       = (uint8_t) (ct->tm_mon + 1),
		.dayOfWeek   = (uint8_t) (ct->tm_wday == 0 ? 7 : ct->tm_wday),
		.day         = (uint8_t) ct->tm_mday,
		.hour        = (uint8_t) ct->tm_hour,
		.minute      = (uint8_t) ct->tm_min,
		.second      = (uint8_t) ct->tm_sec,
		.millisecond = 0,
		.timezone    = mdate_timezone()
	};
	return date;
#endif
}

int8_t mdate_timezone()
{
#if BUILD_IS_SYSTEM_WINDOWS
	TIME_ZONE_INFORMATION timeZone;
	GetTimeZoneInformation(&timeZone);
	return (int8_t) (-timeZone.DaylightBias / 60);
#else
	return (int8_t) (-timezone / 60 / 60);
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
		.tm_mon   = date.month - 1,
		.tm_year  = year,
		.tm_wday  = date.dayOfWeek == 7 ? 0 : date.dayOfWeek,
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
#if BUILD_IS_SYSTEM_WINDOWS
	struct tm tdatei;
	memset(&tdatei, 0, sizeof(tdatei));
	gmtime_s(&tdatei, &((time_t) unix));
	struct tm* tdate = &tdatei;
#else
	time_t t = unix;
	struct tm* tdate = gmtime(&t);
#endif
	mdate_t date = {
		.year        = (uint16_t) (tdate->tm_year + 1900),
		.month       = (uint8_t) (tdate->tm_mon + 1),
		.dayOfWeek   = (uint8_t) (tdate->tm_wday == 0 ? 7 : tdate->tm_wday),
		.day         = (uint8_t) tdate->tm_mday,
		.hour        = (uint8_t) tdate->tm_hour,
		.minute      = (uint8_t) tdate->tm_min,
		.second      = (uint8_t) tdate->tm_sec,
		.millisecond = (uint16_t) milliseconds
	};
	return date;
}

uint64_t mtime_steady_freq()
{
#if BUILD_IS_SYSTEM_WINDOWS
	return s_QPF;
#else
	return 1000000000ULL;
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
	return 1.0;
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
#elif BUILD_IS_SYSTEM_MACOSX
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	return ((uint64_t) tp.tv_sec) * 1000000000 + (uint64_t) tp.tv_nsec;
#else
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return ((uint64_t) tp.tv_sec) * 1000000000 + (uint64_t) tp.tv_nsec;
#endif
}

uint64_t mtime_high_res()
{
	if (!s_HRSupported)
		return mtime_steady();

#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) __rdtsc();
#else
	return (uint64_t) __rdtsc();
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
