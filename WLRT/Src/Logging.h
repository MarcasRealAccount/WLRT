#pragma once

#include "Str.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum WLRTLogSeverity
{
	WLRT_LOG_SEVERITY_ERROR,
	WLRT_LOG_SEVERITY_WARN,
	WLRT_LOG_SEVERITY_DEBUG,
	WLRT_LOG_SEVERITY_INFO
} WLRTLogSeverity;

typedef void (*WLRTLogSinkWriterFn)(void* userdata, bool isError, WLRTStringView buffer);
typedef void (*WLRTLogSinkFlusherFn)(void* userdata, bool isError);

typedef struct WLRTLogSinkData
{
	WLRTLogSeverity      severity;
	WLRTLogSinkWriterFn  writer;
	WLRTLogSinkFlusherFn flusher;
	void*                userdata;
} WLRTLogSinkData;

typedef struct WLRTLoggerData
{
	WLRTString name;
} WLRTLoggerData;

bool WLRTLogSetup();
void WLRTLogCleanup();

bool WLRTLoggerSetup(WLRTLoggerData* logger, WLRTStringView name);
void WLRTLoggerCleanup(WLRTLoggerData* logger);

void WLRTSinkRegister(WLRTLogSinkData* sink);
void WLRTSinkUnregister(WLRTLogSinkData* sink);

void            WLRTLogSetFormat(WLRTStringView format);
WLRTStringView  WLRTLogGetFormat();
void            WLRTLogSetSeverity(WLRTLogSeverity severity);
WLRTLogSeverity WLRTLogGetSeverity();

void WLRTLog(WLRTLogSeverity severity, WLRTStringView message);
void WLRTLogF(WLRTLogSeverity severity, WLRTStringView format, ...);
void WLRTLogFV(WLRTLogSeverity severity, WLRTStringView format, va_list args);
void WLRTError(WLRTStringView message);
void WLRTWarn(WLRTStringView message);
void WLRTDebug(WLRTStringView message);
void WLRTInfo(WLRTStringView message);
void WLRTErrorF(WLRTStringView format, ...);
void WLRTWarnF(WLRTStringView format, ...);
void WLRTDebugF(WLRTStringView format, ...);
void WLRTInfoF(WLRTStringView format, ...);
void WLRTErrorFV(WLRTStringView format, va_list args);
void WLRTWarnFV(WLRTStringView format, va_list args);
void WLRTDebugFV(WLRTStringView format, va_list args);
void WLRTInfoFV(WLRTStringView format, va_list args);

void WLRTLoggerLog(const WLRTLoggerData* logger, WLRTLogSeverity severity, WLRTStringView message);
void WLRTLoggerLogF(const WLRTLoggerData* logger, WLRTLogSeverity severity, WLRTStringView format, ...);
void WLRTLoggerLogFV(const WLRTLoggerData* logger, WLRTLogSeverity severity, WLRTStringView format, va_list args);
void WLRTLoggerError(const WLRTLoggerData* logger, WLRTStringView message);
void WLRTLoggerWarn(const WLRTLoggerData* logger, WLRTStringView message);
void WLRTLoggerDebug(const WLRTLoggerData* logger, WLRTStringView message);
void WLRTLoggerInfo(const WLRTLoggerData* logger, WLRTStringView message);
void WLRTLoggerErrorF(const WLRTLoggerData* logger, WLRTStringView format, ...);
void WLRTLoggerWarnF(const WLRTLoggerData* logger, WLRTStringView format, ...);
void WLRTLoggerDebugF(const WLRTLoggerData* logger, WLRTStringView format, ...);
void WLRTLoggerInfoF(const WLRTLoggerData* logger, WLRTStringView format, ...);
void WLRTLoggerErrorFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args);
void WLRTLoggerWarnFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args);
void WLRTLoggerDebugFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args);
void WLRTLoggerInfoFV(const WLRTLoggerData* logger, WLRTStringView format, va_list args);