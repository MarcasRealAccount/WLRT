#include "Logging.h"
#include "Build.h"
#include "DynArray.h"
#include "Threading.h"

typedef struct WLRTLogData
{
	WLRTMutex       mutex;
	WLRTString      format;
	WLRTLogSeverity severity;
	WLRTDynArray    sinks;
} WLRTLogData;

static WLRTLogData s_Log;

static WLRTStringView WLRTLogSeverityToString(WLRTLogSeverity severity)
{
	switch (severity)
	{
	case WLRT_LOG_SEVERITY_ERROR: return WLRTStringViewCreate("Error", 5);
	case WLRT_LOG_SEVERITY_WARN: return WLRTStringViewCreate("Warn", 4);
	case WLRT_LOG_SEVERITY_DEBUG: return WLRTStringViewCreate("Debug", 5);
	case WLRT_LOG_SEVERITY_INFO: return WLRTStringViewCreate("Info", 4);
	default: return WLRTStringViewCreate(NULL, 0);
	}
}

static WLRTStringView WLRTLogSeverityColor(WLRTLogSeverity severity)
{
	switch (severity)
	{
	case WLRT_LOG_SEVERITY_ERROR: return WLRTStringViewCreate("\033[31m", 5);
	case WLRT_LOG_SEVERITY_WARN: return WLRTStringViewCreate("\033[33m", 5);
	case WLRT_LOG_SEVERITY_DEBUG: return WLRTStringViewCreate("\033[34m", 5);
	case WLRT_LOG_SEVERITY_INFO: return WLRTStringViewCreate("\033[37m", 5);
	default: return WLRTStringViewCreate("\033[39m", 5);
	}
}

static void WLRTLogWrite(WLRTLogSeverity severity, WLRTStringView buffer)
{
	bool isError = severity == WLRT_LOG_SEVERITY_ERROR;
	for (size_t i = 0; i < s_Log.sinks.size; ++i)
	{
		WLRTLogSinkData* sink = *(WLRTLogSinkData**) WLRTDynArrayGet(&s_Log.sinks, i);
		if (severity <= sink->severity)
			sink->writer(sink->userdata, isError, buffer);
	}
}

static void WLRTLogFlush(WLRTLogSeverity severity)
{
	bool isError = severity == WLRT_LOG_SEVERITY_ERROR;
	for (size_t i = 0; i < s_Log.sinks.size; ++i)
	{
		WLRTLogSinkData* sink = *(WLRTLogSinkData**) WLRTDynArrayGet(&s_Log.sinks, i);
		if (severity <= sink->severity)
			sink->flusher(sink->userdata, isError);
	}
}

bool WLRTLogSetup()
{
	if (!WLRTMutexSetup(&s_Log.mutex))
		return false;
	s_Log.format = WLRTStringCreate("[%T %e][%t] %n %^%s%$: %v", ~0ULL);
#if BUILD_IS_CONFIG_DEBUG
	s_Log.severity = WLRT_LOG_SEVERITY_INFO;
#elif BUILD_IS_CONFIG_DIST
	s_Log.severity = WLRT_LOG_SEVERITY_WARN;
#endif
	if (!WLRTDynArraySetup(&s_Log.sinks, 32, sizeof(WLRTLogSinkData*)))
	{
		WLRTLogCleanup();
		return false;
	}
	return true;
}

void WLRTLogCleanup()
{
	WLRTMutexCleanup(&s_Log.mutex);
	WLRTStringCleanup(&s_Log.format);
	WLRTDynArrayCleanup(&s_Log.sinks);
}

bool WLRTLoggerSetup(WLRTLoggerData* logger, WLRTStringView name)
{
	logger->name = WLRTStringCreate(name.string, name.length);
	return true;
}

void WLRTLoggerCleanup(WLRTLoggerData* logger)
{
	WLRTStringCleanup(&logger->name);
}

void WLRTSinkRegister(WLRTLogSinkData* sink)
{
	WLRTDynArrayPushBack(&s_Log.sinks, &sink);
}

void WLRTSinkUnregister(WLRTLogSinkData* sink)
{
	for (size_t i = 0; i < s_Log.sinks.size; ++i)
	{
		WLRTLogSinkData* setSink = *(WLRTLogSinkData**) WLRTDynArrayGet(&s_Log.sinks, i);
		if (setSink == sink)
		{
			WLRTDynArrayErase(&s_Log.sinks, i);
			break;
		}
	}
}

void WLRTLogSetFormat(WLRTStringView format)
{
	WLRTStringAssign(&s_Log.format, format);
}

WLRTStringView WLRTLogGetFormat()
{
	return WLRTStringSubstr(&s_Log.format, 0, ~0ULL);
}

void WLRTLogSetSeverity(WLRTLogSeverity severity)
{
	s_Log.severity = severity;
}

WLRTLogSeverity WLRTLogGetSeverity()
{
	return s_Log.severity;
}

void WLRTLog(WLRTLogSeverity severity, WLRTStringView message)
{
	WLRTLoggerLog(NULL, severity, message);
}

void WLRTLogF(WLRTLogSeverity severity, WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLogFV(severity, format, args);
	va_end(args);
}

void WLRTLogFV(WLRTLogSeverity severity, WLRTStringView format, va_list args)
{
	WLRTString value = WLRTStringFormatV(format, args);
	WLRTLoggerLog(NULL, severity, WLRTStringSubstr(&value, 0, ~0ULL));
	WLRTStringCleanup(&value);
}

void WLRTError(WLRTStringView message)
{
	WLRTLog(WLRT_LOG_SEVERITY_ERROR, message);
}

void WLRTWarn(WLRTStringView message)
{
	WLRTLog(WLRT_LOG_SEVERITY_WARN, message);
}

void WLRTDebug(WLRTStringView message)
{
	WLRTLog(WLRT_LOG_SEVERITY_DEBUG, message);
}

void WLRTInfo(WLRTStringView message)
{
	WLRTLog(WLRT_LOG_SEVERITY_INFO, message);
}

void WLRTErrorF(WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLogFV(WLRT_LOG_SEVERITY_ERROR, format, args);
	va_end(args);
}

void WLRTWarnF(WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLogFV(WLRT_LOG_SEVERITY_WARN, format, args);
	va_end(args);
}

void WLRTDebugF(WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLogFV(WLRT_LOG_SEVERITY_DEBUG, format, args);
	va_end(args);
}

void WLRTInfoF(WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLogFV(WLRT_LOG_SEVERITY_INFO, format, args);
	va_end(args);
}

void WLRTErrorFV(WLRTStringView format, va_list args)
{
	WLRTLogFV(WLRT_LOG_SEVERITY_ERROR, format, args);
}

void WLRTWarnFV(WLRTStringView format, va_list args)
{
	WLRTLogFV(WLRT_LOG_SEVERITY_WARN, format, args);
}

void WLRTDebugFV(WLRTStringView format, va_list args)
{
	WLRTLogFV(WLRT_LOG_SEVERITY_DEBUG, format, args);
}

void WLRTInfoFV(WLRTStringView format, va_list args)
{
	WLRTLogFV(WLRT_LOG_SEVERITY_INFO, format, args);
}

void WLRTLoggerLog(const WLRTLoggerData* logger, WLRTLogSeverity severity, WLRTStringView message)
{
	WLRTDate time = WLRTGetLocalDate();

	WLRTString tempBuffer;
	WLRTStringSetup(&tempBuffer, 8);

	WLRTStringView format = WLRTStringSubstr(&s_Log.format, 0, ~0ULL);
	size_t         offset = 0;
	while (offset < format.length)
	{
		size_t end = offset;
		while (end < format.length && format.string[end] != '%')
			++end;
		if (end >= format.length && offset != format.length - 1)
		{
			WLRTLogWrite(severity, WLRTStringViewSubstr(format, offset, ~0ULL));
			break;
		}
		if ((end - offset) > 0)
			WLRTLogWrite(severity, WLRTStringViewSubstr(format, offset, end));
		if (end + 1 >= format.length)
			break;
		switch (format.string[end + 1])
		{
		case 'H':
		{
			WLRTStringResize(&tempBuffer, 2);
			char*   buf = WLRTStringData(&tempBuffer);
			uint8_t u, l;
			u      = time.hour / 10;
			l      = time.hour % 10;
			buf[0] = '0' + (char) u;
			buf[1] = '0' + (char) l;
			WLRTLogWrite(severity, WLRTStringSubstr(&tempBuffer, 0, 2));
			break;
		}
		case 'M':
		{
			WLRTStringResize(&tempBuffer, 2);
			char*   buf = WLRTStringData(&tempBuffer);
			uint8_t u, l;
			u      = time.minute / 10;
			l      = time.minute % 10;
			buf[0] = '0' + (char) u;
			buf[1] = '0' + (char) l;
			WLRTLogWrite(severity, WLRTStringSubstr(&tempBuffer, 0, 2));
			break;
		}
		case 'S':
		{
			WLRTStringResize(&tempBuffer, 2);
			char*   buf = WLRTStringData(&tempBuffer);
			uint8_t u, l;
			u      = time.second / 10;
			l      = time.second % 10;
			buf[0] = '0' + (char) u;
			buf[1] = '0' + (char) l;
			WLRTLogWrite(severity, WLRTStringSubstr(&tempBuffer, 0, 2));
			break;
		}
		case 'e':
		{
			WLRTStringResize(&tempBuffer, 3);
			char*    buf = WLRTStringData(&tempBuffer);
			uint16_t u, l;
			u      = time.millisecond / 10;
			l      = time.millisecond % 10;
			buf[2] = '0' + (char) l;
			l      = u % 10;
			u      = u / 10;
			buf[0] = '0' + (char) u;
			buf[1] = '0' + (char) l;
			WLRTLogWrite(severity, WLRTStringSubstr(&tempBuffer, 0, 3));
			break;
		}
		case 'T':
		{
			WLRTStringResize(&tempBuffer, 8);
			char* buf = WLRTStringData(&tempBuffer);
			buf[2]    = ':';
			buf[5]    = ':';
			uint8_t u, l;
			u      = time.hour / 10;
			l      = time.hour % 10;
			buf[0] = '0' + (char) u;
			buf[1] = '0' + (char) l;
			u      = time.minute / 10;
			l      = time.minute % 10;
			buf[3] = '0' + (char) u;
			buf[4] = '0' + (char) l;
			u      = time.second / 10;
			l      = time.second % 10;
			buf[6] = '0' + (char) u;
			buf[7] = '0' + (char) l;
			WLRTLogWrite(severity, WLRTStringSubstr(&tempBuffer, 0, 8));
			break;
		}
		case 't':
		{
			uint32_t tid = WLRTGetCurrentThreadId();
			WLRTStringResize(&tempBuffer, 10);
			char*    buf = WLRTStringData(&tempBuffer);
			uint32_t u   = tid, l;
			size_t   i   = 10;
			for (; i > 0; --i)
			{
				l          = u % 10;
				u          = u / 10;
				buf[i - 1] = '0' + (char) l;
				if (u == 0)
					break;
			}
			WLRTLogWrite(severity, WLRTStringSubstr(&tempBuffer, i - 1, 10));
			break;
		}
		case 'n':
			WLRTLogWrite(severity, logger ? WLRTStringSubstr(&logger->name, 0, ~0ULL) : WLRTStringViewCreate("WLRT", ~0ULL));
			break;
		case 's':
			WLRTLogWrite(severity, WLRTLogSeverityToString(severity));
			break;
		case 'v':
			WLRTLogWrite(severity, message);
			break;
		case '^':
			WLRTLogWrite(severity, WLRTLogSeverityColor(severity));
			break;
		case '$':
			WLRTLogWrite(severity, WLRTStringViewCreate("\033[39m", 5));
			break;
		case '%':
			WLRTLogWrite(severity, WLRTStringViewCreate("%", 1));
			break;
		}
		offset = end + 2;
	}
	WLRTLogWrite(severity, WLRTStringViewCreate("\n", 1));
	WLRTLogFlush(severity);
}

void WLRTLoggerLogF(const WLRTLoggerData* logger, WLRTLogSeverity severity, WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLoggerLogFV(logger, severity, format, args);
	va_end(args);
}

void WLRTLoggerLogFV(const WLRTLoggerData* logger, WLRTLogSeverity severity, WLRTStringView format, va_list args)
{
	WLRTString value = WLRTStringFormatV(format, args);
	WLRTLoggerLog(logger, severity, WLRTStringSubstr(&value, 0, ~0ULL));
	WLRTStringCleanup(&value);
}

void WLRTLoggerError(const WLRTLoggerData* logger, WLRTStringView message)
{
	WLRTLoggerLog(logger, WLRT_LOG_SEVERITY_ERROR, message);
}

void WLRTLoggerWarn(const WLRTLoggerData* logger, WLRTStringView message)
{
	WLRTLoggerLog(logger, WLRT_LOG_SEVERITY_WARN, message);
}

void WLRTLoggerDebug(const WLRTLoggerData* logger, WLRTStringView message)
{
	WLRTLoggerLog(logger, WLRT_LOG_SEVERITY_DEBUG, message);
}

void WLRTLoggerInfo(const WLRTLoggerData* logger, WLRTStringView message)
{
	WLRTLoggerLog(logger, WLRT_LOG_SEVERITY_INFO, message);
}

void WLRTLoggerErrorF(const WLRTLoggerData* logger, WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_ERROR, format, args);
	va_end(args);
}

void WLRTLoggerWarnF(const WLRTLoggerData* logger, WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_WARN, format, args);
	va_end(args);
}

void WLRTLoggerDebugF(const WLRTLoggerData* logger, WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_DEBUG, format, args);
	va_end(args);
}

void WLRTLoggerInfoF(const WLRTLoggerData* logger, WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_INFO, format, args);
	va_end(args);
}

void WLRTLoggerErrorFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args)
{
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_ERROR, format, args);
}

void WLRTLoggerWarnFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args)
{
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_WARN, format, args);
}

void WLRTLoggerDebugFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args)
{
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_DEBUG, format, args);
}

void WLRTLoggerInfoFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args)
{
	WLRTLoggerLogFV(logger, WLRT_LOG_SEVERITY_INFO, format, args);
}