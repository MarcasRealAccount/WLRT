#pragma once

#include "str.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum mlog_severity_e
{
	mlog_severity_error = 0,
	mlog_severity_warn,
	mlog_severity_debug,
	mlog_severity_info
} mlog_severity_e;

typedef void (*mlog_sink_write_func_t)(void* data, bool isError, mstringview_t buffer);
typedef void (*mlog_sink_flush_func_t)(void* data, bool isError);

typedef struct mlog_sink_t
{
	mlog_severity_e        severity;
	mlog_sink_write_func_t write;
	mlog_sink_flush_func_t flush;
	void*                  data;
} mlog_sink_t;

typedef struct mlogger_t
{
	mstring_t name;
} mlogger_t;

mlog_sink_t mlog_stdout_sink();

mlogger_t  mlogger(mstringview_t name);
mlogger_t* mlogger_new(mstringview_t name);
void       mlogger_del(mlogger_t* self);
bool       mlogger_cstr(mlogger_t* self, mstringview_t name);
void       mlogger_dstr(mlogger_t* self);

bool            mlog_init();
void            mlog_deinit();
void            mlog_set_format(mstringview_t format);
mstringview_t   mlog_get_format();
void            mlog_set_severity(mlog_severity_e severity);
mlog_severity_e mlog_get_severity();
bool            mlog_sink_register(mlog_sink_t* sink);
void            mlog_sink_unregister(mlog_sink_t* sink);

void mlog(mlog_severity_e severity, mstringview_t message);
void mlogf(mlog_severity_e severity, mstringview_t format, ...);
void mlogfv(mlog_severity_e severity, mstringview_t format, va_list args);
void merror(mstringview_t message);
void merrorf(mstringview_t format, ...);
void merrorfv(mstringview_t format, va_list args);
void mwarn(mstringview_t message);
void mwarnf(mstringview_t format, ...);
void mwarnfv(mstringview_t format, va_list args);
void mdebug(mstringview_t message);
void mdebugf(mstringview_t format, ...);
void mdebugfv(mstringview_t format, va_list args);
void minfo(mstringview_t message);
void minfof(mstringview_t format, ...);
void minfofv(mstringview_t format, va_list args);

void mllog(const mlogger_t* logger, mlog_severity_e severity, mstringview_t message);
void mllogf(const mlogger_t* logger, mlog_severity_e severity, mstringview_t format, ...);
void mllogfv(const mlogger_t* logger, mlog_severity_e severity, mstringview_t format, va_list args);
void mlerror(const mlogger_t* logger, mstringview_t message);
void mlerrorf(const mlogger_t* logger, mstringview_t format, ...);
void mlerrorfv(const mlogger_t* logger, mstringview_t format, va_list args);
void mlwarn(const mlogger_t* logger, mstringview_t message);
void mlwarnf(const mlogger_t* logger, mstringview_t format, ...);
void mlwarnfv(const mlogger_t* logger, mstringview_t format, va_list args);
void mldebug(const mlogger_t* logger, mstringview_t message);
void mldebugf(const mlogger_t* logger, mstringview_t format, ...);
void mldebugfv(const mlogger_t* logger, mstringview_t format, va_list args);
void mlinfo(const mlogger_t* logger, mstringview_t message);
void mlinfof(const mlogger_t* logger, mstringview_t format, ...);
void mlinfofv(const mlogger_t* logger, mstringview_t format, va_list args);