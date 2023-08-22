#include "log.h"
#include "build.h"
#include "dynarray.h"
#include "mem.h"
#include "thread.h"
#include "tim.h"

#include <stdio.h>

typedef enum mlog_format_part_e
{
	mlog_format_part_str = 0,
	mlog_format_part_value,          // %v
	mlog_format_part_thread_id,      // %t
	mlog_format_part_logger_name,    // %n
	mlog_format_part_severity,       // %L
	mlog_format_part_severity_short, // %l
	mlog_format_part_date,           //	%D
	mlog_format_part_date_short,     //	%C
	mlog_format_part_time,           //	%T
	mlog_format_part_timezone,       //	%z
	mlog_format_part_date_and_time,  //	%c
	mlog_format_part_year,           // %Y
	mlog_format_part_year_short,     // %y
	mlog_format_part_month,          // %B
	mlog_format_part_month_short,    // %b
	mlog_format_part_weekday,        // %A
	mlog_format_part_weekday_short,  // %a
	mlog_format_part_month_num,      // %m
	mlog_format_part_day_num,        // %d
	mlog_format_part_hour,           // %H
	mlog_format_part_minute,         // %M
	mlog_format_part_second,         //	%S
	mlog_format_part_millisecond,    //	%e
	mlog_format_part_since_epoch,    //	%E
	mlog_format_part_start_color,    //	%^
	mlog_format_part_end_color       //	%$
} mlog_format_part_e;

typedef struct mlog_format_part_t
{
	mlog_format_part_e part;
	mstring_t          string;
} mlog_format_part_t;

typedef struct mlog_data_t
{
	mstring_t       format;
	mdynarray_t     compiledFormat;
	mlog_severity_e severity;
	mdynarray_t     sinks;
} mlog_data_t;

static mlog_data_t s_Log;

static mdynarray_t mlog_compile_format(mstringview_t format)
{
	mdynarray_t arr = mdynarray(32, sizeof(mlog_format_part_t));

	size_t offset = 0;
	while (offset < format.length)
	{
		size_t start = mstringview_find_first_of(format, mstringviewcstr("%"), offset);
		size_t len   = start - offset;
		if (len > 0)
		{
			mlog_format_part_t part;
			part.part   = mlog_format_part_str;
			part.string = mstringv(mstringview_substr(format, offset, len));
			mdynarray_pushback(&arr, &part);
		}
		if (start + 1 >= format.length)
			break;
		offset = start + 2;
		mlog_format_part_t part;
		part.part   = mlog_format_part_str;
		part.string = mstring(0);
		switch (format.string[start + 1])
		{
		case 'v': part.part = mlog_format_part_value; break;
		case 't': part.part = mlog_format_part_thread_id; break;
		case 'n': part.part = mlog_format_part_logger_name; break;
		case 'L': part.part = mlog_format_part_severity; break;
		case 'l': part.part = mlog_format_part_severity_short; break;
		case 'D': part.part = mlog_format_part_date; break;
		case 'C': part.part = mlog_format_part_date_short; break;
		case 'T': part.part = mlog_format_part_time; break;
		case 'z': part.part = mlog_format_part_timezone; break;
		case 'c': part.part = mlog_format_part_date_and_time; break;
		case 'Y': part.part = mlog_format_part_year; break;
		case 'y': part.part = mlog_format_part_year_short; break;
		case 'B': part.part = mlog_format_part_month; break;
		case 'b': part.part = mlog_format_part_month_short; break;
		case 'A': part.part = mlog_format_part_weekday; break;
		case 'a': part.part = mlog_format_part_weekday_short; break;
		case 'm': part.part = mlog_format_part_month_num; break;
		case 'd': part.part = mlog_format_part_day_num; break;
		case 'H': part.part = mlog_format_part_hour; break;
		case 'M': part.part = mlog_format_part_minute; break;
		case 'S': part.part = mlog_format_part_second; break;
		case 'e': part.part = mlog_format_part_millisecond; break;
		case 'E': part.part = mlog_format_part_since_epoch; break;
		case '^': part.part = mlog_format_part_start_color; break;
		case '$': part.part = mlog_format_part_end_color; break;
		case '%': mstring_pushback(&part.string, '%'); break;
		default: continue;
		}
		mdynarray_pushback(&arr, &part);
	}
	return arr;
}

static mstringview_t mlog_severity_to_string(mlog_severity_e severity)
{
	switch (severity)
	{
	case mlog_severity_error: return mstringviewcstr("Error");
	case mlog_severity_warn: return mstringviewcstr("Warn");
	case mlog_severity_debug: return mstringviewcstr("Debug");
	case mlog_severity_info: return mstringviewcstr("Info");
	default: return mstringviewcstr(NULL);
	}
}

static mstringview_t mlog_severity_to_short_string(mlog_severity_e severity)
{
	switch (severity)
	{
	case mlog_severity_error: return mstringviewcstr("E");
	case mlog_severity_warn: return mstringviewcstr("W");
	case mlog_severity_debug: return mstringviewcstr("D");
	case mlog_severity_info: return mstringviewcstr("I");
	default: return mstringviewcstr(NULL);
	}
}

static mstringview_t mlog_severity_color(mlog_severity_e severity)
{
	switch (severity)
	{
	case mlog_severity_error: return mstringviewcstr("\033[31m");
	case mlog_severity_warn: return mstringviewcstr("\033[33m");
	case mlog_severity_debug: return mstringviewcstr("\033[34m");
	case mlog_severity_info: return mstringviewcstr("\033[37m");
	default: return mstringviewcstr("\033[39m");
	}
}

static void mlog_write(mlog_severity_e severity, mstringview_t buffer)
{
	bool isError = severity == mlog_severity_error;
	for (size_t i = 0; i < s_Log.sinks.size; ++i)
	{
		mlog_sink_t* sink = *(mlog_sink_t**) mdynarray_get(&s_Log.sinks, i);
		if (severity <= sink->severity)
			sink->write(sink->data, isError, buffer);
	}
}

static void mlog_flush(mlog_severity_e severity)
{
	bool isError = severity == mlog_severity_error;
	for (size_t i = 0; i < s_Log.sinks.size; ++i)
	{
		mlog_sink_t* sink = *(mlog_sink_t**) mdynarray_get(&s_Log.sinks, i);
		if (severity <= sink->severity)
			sink->flush(sink->data, isError);
	}
}

static char s_HexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

static void mlog_log(const mlogger_t* logger, mlog_severity_e severity, mstringview_t message)
{
	mdate_t   now    = mdate_local_now();
	mstring_t buffer = mstring(0);
	for (size_t i = 0; i < s_Log.compiledFormat.size; ++i)
	{
		mlog_format_part_t* part = (mlog_format_part_t*) mdynarray_get(&s_Log.compiledFormat, i);

		switch (part->part)
		{
		case mlog_format_part_str: mstring_append(&buffer, mstringviews(&part->string)); break;
		case mlog_format_part_value: mstring_append(&buffer, message); break;
		case mlog_format_part_thread_id:
		{
			mthreadid_t tid = mthread_current_id();
			char        buf[8];
			for (uint8_t j = 0; j < 8; ++j)
				buf[j] = s_HexDigits[(tid >> (24 - j * 4)) & 0xF];
			mstring_append(&buffer, mstringview(buf, 8));
			break;
		}
		case mlog_format_part_logger_name: mstring_append(&buffer, logger ? mstringviews(&logger->name) : mstringviewcstr("WLRT")); break;
		case mlog_format_part_severity: mstring_append(&buffer, mlog_severity_to_string(severity)); break;
		case mlog_format_part_severity_short: mstring_append(&buffer, mlog_severity_to_short_string(severity)); break;
		case mlog_format_part_date:
		{
			mstring_append(&buffer, mday_of_week_to_short_string(now.dayOfWeek));
			mstring_pushback(&buffer, ' ');
			mstring_append(&buffer, mmonth_to_short_string(now.month));
			mstring_pushback(&buffer, ' ');
			char buf[4];
			buf[0] = '0' + (char) (now.day / 10);
			buf[1] = '0' + (char) (now.day % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, ' ');
			uint16_t r, d;
			r      = now.year / 10;
			d      = now.year % 10;
			buf[3] = '0' + (char) (d);
			d      = r % 10;
			r      = r / 10;
			buf[2] = '0' + (char) (d);
			buf[0] = '0' + (char) (r / 10);
			buf[1] = '0' + (char) (r % 10);
			mstring_append(&buffer, mstringview(buf, 4));
			break;
		}
		case mlog_format_part_date_short:
		{
			char buf[4];
			buf[0] = '0' + (char) (now.day / 10);
			buf[1] = '0' + (char) (now.day % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, '/');
			buf[0] = '0' + (char) (now.month / 10);
			buf[1] = '0' + (char) (now.month % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, '/');
			uint16_t r, d;
			r      = now.year / 10;
			d      = now.year % 10;
			buf[3] = '0' + (char) (d);
			d      = r % 10;
			r      = r / 10;
			buf[2] = '0' + (char) (d);
			buf[0] = '0' + (char) (r / 10);
			buf[1] = '0' + (char) (r % 10);
			mstring_append(&buffer, mstringview(buf, 4));
			break;
		}
		case mlog_format_part_time:
		{
			char buf[2];
			buf[0] = '0' + (char) (now.hour / 10);
			buf[1] = '0' + (char) (now.hour % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, ':');
			buf[0] = '0' + (char) (now.minute / 10);
			buf[1] = '0' + (char) (now.minute % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, ':');
			buf[0] = '0' + (char) (now.second / 10);
			buf[1] = '0' + (char) (now.second % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			break;
		}
		case mlog_format_part_timezone:
		{
			char buf[6];
			buf[0] = '+';
			buf[1] = '0' + (char) (now.timezone / 10);
			buf[2] = '0' + (char) (now.timezone % 10);
			buf[3] = ':';
			buf[4] = '0';
			buf[5] = '0';
			mstring_append(&buffer, mstringview(buf, 6));
			break;
		}
		case mlog_format_part_date_and_time:
		{
			mstring_append(&buffer, mday_of_week_to_short_string(now.dayOfWeek));
			mstring_pushback(&buffer, ' ');
			mstring_append(&buffer, mmonth_to_short_string(now.month));
			mstring_pushback(&buffer, ' ');
			char buf[4];
			buf[0] = '0' + (char) (now.day / 10);
			buf[1] = '0' + (char) (now.day % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, ' ');
			buf[0] = '0' + (char) (now.hour / 10);
			buf[1] = '0' + (char) (now.hour % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, ':');
			buf[0] = '0' + (char) (now.minute / 10);
			buf[1] = '0' + (char) (now.minute % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			mstring_pushback(&buffer, ':');
			buf[0] = '0' + (char) (now.second / 10);
			buf[1] = '0' + (char) (now.second % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			uint16_t r, d;
			r      = now.year / 10;
			d      = now.year % 10;
			buf[3] = '0' + (char) (d);
			d      = r % 10;
			r      = r / 10;
			buf[2] = '0' + (char) (d);
			buf[0] = '0' + (char) (r / 10);
			buf[1] = '0' + (char) (r % 10);
			mstring_append(&buffer, mstringview(buf, 4));
			break;
		}
		case mlog_format_part_year:
		{
			char     buf[4];
			uint16_t r, d;
			r      = now.year / 10;
			d      = now.year % 10;
			buf[3] = '0' + (char) (d);
			d      = r % 10;
			r      = r / 10;
			buf[2] = '0' + (char) (d);
			buf[0] = '0' + (char) (r / 10);
			buf[1] = '0' + (char) (r % 10);
			mstring_append(&buffer, mstringview(buf, 4));
			break;
		}
		case mlog_format_part_year_short:
		{
			char buf[2];
			buf[0] = '0' + (char) ((now.year / 10) % 10);
			buf[1] = '0' + (char) (now.year % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			break;
		}
		case mlog_format_part_month: mstring_append(&buffer, mmonth_to_string(now.month)); break;
		case mlog_format_part_month_short: mstring_append(&buffer, mmonth_to_short_string(now.month)); break;
		case mlog_format_part_weekday: mstring_append(&buffer, mday_of_week_to_string(now.dayOfWeek)); break;
		case mlog_format_part_weekday_short: mstring_append(&buffer, mday_of_week_to_short_string(now.dayOfWeek)); break;
		case mlog_format_part_month_num:
		{
			char buf[2];
			buf[0] = '0' + (char) (now.month / 10);
			buf[1] = '0' + (char) (now.month % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			break;
		}
		case mlog_format_part_day_num:
		{
			char buf[2];
			buf[0] = '0' + (char) (now.day / 10);
			buf[1] = '0' + (char) (now.day % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			break;
		}
		case mlog_format_part_hour:
		{
			char buf[2];
			buf[0] = '0' + (char) (now.hour / 10);
			buf[1] = '0' + (char) (now.hour % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			break;
		}
		case mlog_format_part_minute:
		{
			char buf[2];
			buf[0] = '0' + (char) (now.minute / 10);
			buf[1] = '0' + (char) (now.minute % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			break;
		}
		case mlog_format_part_second:
		{
			char buf[2];
			buf[0] = '0' + (char) (now.second / 10);
			buf[1] = '0' + (char) (now.second % 10);
			mstring_append(&buffer, mstringview(buf, 2));
			break;
		}
		case mlog_format_part_millisecond:
		{
			char     buf[4];
			uint16_t r, d;
			r      = now.millisecond / 10;
			d      = now.millisecond % 10;
			buf[3] = '0' + (char) (d);
			d      = r % 10;
			r      = r / 10;
			buf[2] = '0' + (char) (d);
			buf[0] = '0' + (char) (r / 10);
			buf[1] = '0' + (char) (r % 10);
			mstring_append(&buffer, mstringview(buf, 4));
			break;
		}
		case mlog_format_part_since_epoch:
		{
			int64_t unixTime = mdate_to_unix(now);
			char    buf[20];
			uint8_t j = 0;
			for (; j < 20; ++j)
			{
				buf[19 - j] = '0' + (char) (unixTime % 10);
				if ((unixTime /= 10) == 0)
					break;
			}
			mstring_append(&buffer, mstringview(buf + 19 - j, j));
			break;
		}
		case mlog_format_part_start_color: mstring_append(&buffer, mlog_severity_color(severity)); break;
		case mlog_format_part_end_color: mstring_append(&buffer, mstringviewcstr("\033[39m")); break;
		}
	}
	mstring_pushback(&buffer, '\n');
	mlog_write(severity, mstringviews(&buffer));
	mlog_flush(severity);
	mstring_dstr(&buffer);
}

static void mlog_stdout_write(void* data, bool isError, mstringview_t buffer)
{
	(void) data;
	fwrite(buffer.string, sizeof(char), buffer.length, isError ? stderr : stdout);
}

static void mlog_stdout_flush(void* data, bool isError)
{
	(void) data;
	fflush(isError ? stderr : stdout);
}

mlog_sink_t mlog_stdout_sink()
{
	mlog_sink_t sink = {
		.severity = mlog_severity_info,
		.write    = &mlog_stdout_write,
		.flush    = &mlog_stdout_flush,
		.data     = NULL
	};
	return sink;
}

mlogger_t mlogger(mstringview_t name)
{
	mlogger_t logger;
	memset(&logger, 0, sizeof(logger));
	mlogger_cstr(&logger, name);
	return logger;
}

mlogger_t* mlogger_new(mstringview_t name)
{
	mlogger_t* logger = (mlogger_t*) mmalloc(sizeof(mlogger_t));
	mlogger_cstr(logger, name);
	return logger;
}

void mlogger_del(mlogger_t* self)
{
	if (!self)
		return;
	mlogger_dstr(self);
	mfree(self);
}

bool mlogger_cstr(mlogger_t* self, mstringview_t name)
{
	if (!self)
		return false;
	self->name = mstringv(name);
	return true;
}

void mlogger_dstr(mlogger_t* self)
{
	if (!self)
		return;
	mstring_dstr(&self->name);
}

bool mlog_init()
{
	s_Log.format = mstring(0);
#if BUILD_IS_CONFIG_DEBUG
	s_Log.severity = mlog_severity_info;
#else
	s_Log.severity = mlog_severity_warn;
#endif
	mdynarray_cstr(&s_Log.sinks, 32, sizeof(mlog_sink_t*));
	mlog_set_format(mstringviewcstr("[%T %e][%t] %n %^%L%$: %v"));
	return true;
}

void mlog_deinit()
{
	for (size_t i = 0; i < s_Log.compiledFormat.size; ++i)
	{
		mlog_format_part_t* part = (mlog_format_part_t*) mdynarray_get(&s_Log.compiledFormat, i);
		mstring_dstr(&part->string);
	}
	mstring_dstr(&s_Log.format);
	mdynarray_dstr(&s_Log.compiledFormat);
	mdynarray_dstr(&s_Log.sinks);
}

void mlog_set_format(mstringview_t format)
{
	mstring_assign(&s_Log.format, format);
	s_Log.compiledFormat = mlog_compile_format(mstringviews(&s_Log.format));
}

mstringview_t mlog_get_format()
{
	return mstringviews(&s_Log.format);
}

void mlog_set_severity(mlog_severity_e severity)
{
	s_Log.severity = severity;
}

mlog_severity_e mlog_get_severity()
{
	return s_Log.severity;
}

bool mlog_sink_register(mlog_sink_t* sink)
{
	return mdynarray_pushback(&s_Log.sinks, &sink);
}

void mlog_sink_unregister(mlog_sink_t* sink)
{
	for (size_t i = 0; i < s_Log.sinks.size; ++i)
	{
		mlog_sink_t* setSink = *(mlog_sink_t**) mdynarray_get(&s_Log.sinks, i);
		if (setSink != sink)
			continue;
		mdynarray_erase(&s_Log.sinks, i, 1);
		break;
	}
}

void mlog(mlog_severity_e severity, mstringview_t message)
{
	mlog_log(NULL, severity, message);
}

void mlogf(mlog_severity_e severity, mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(NULL, severity, format, args);
	va_end(args);
}

void mlogfv(mlog_severity_e severity, mstringview_t format, va_list args)
{
	mllogfv(NULL, severity, format, args);
}

void merror(mstringview_t message)
{
	mlog_log(NULL, mlog_severity_error, message);
}

void merrorf(mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(NULL, mlog_severity_error, format, args);
	va_end(args);
}

void merrorfv(mstringview_t format, va_list args)
{
	mllogfv(NULL, mlog_severity_error, format, args);
}

void mwarn(mstringview_t message)
{
	mlog_log(NULL, mlog_severity_warn, message);
}

void mwarnf(mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(NULL, mlog_severity_warn, format, args);
	va_end(args);
}

void mwarnfv(mstringview_t format, va_list args)
{
	mllogfv(NULL, mlog_severity_warn, format, args);
}

void mdebug(mstringview_t message)
{
	mlog_log(NULL, mlog_severity_debug, message);
}

void mdebugf(mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(NULL, mlog_severity_debug, format, args);
	va_end(args);
}

void mdebugfv(mstringview_t format, va_list args)
{
	mllogfv(NULL, mlog_severity_debug, format, args);
}

void minfo(mstringview_t message)
{
	mlog_log(NULL, mlog_severity_info, message);
}

void minfof(mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(NULL, mlog_severity_info, format, args);
	va_end(args);
}

void minfofv(mstringview_t format, va_list args)
{
	mllogfv(NULL, mlog_severity_info, format, args);
}

void mllog(const mlogger_t* logger, mlog_severity_e severity, mstringview_t message)
{
	mlog_log(logger, severity, message);
}

void mllogf(const mlogger_t* logger, mlog_severity_e severity, mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(logger, severity, format, args);
	va_end(args);
}

void mllogfv(const mlogger_t* logger, mlog_severity_e severity, mstringview_t format, va_list args)
{
	mstring_t message = mstring_format_va(format, args);
	mlog_log(logger, severity, mstringviews(&message));
	mstring_dstr(&message);
}

void mlerror(const mlogger_t* logger, mstringview_t message)
{
	mlog_log(logger, mlog_severity_error, message);
}

void mlerrorf(const mlogger_t* logger, mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(logger, mlog_severity_error, format, args);
	va_end(args);
}

void mlerrorfv(const mlogger_t* logger, mstringview_t format, va_list args)
{
	mllogfv(logger, mlog_severity_error, format, args);
}

void mlwarn(const mlogger_t* logger, mstringview_t message)
{
	mlog_log(logger, mlog_severity_warn, message);
}

void mlwarnf(const mlogger_t* logger, mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(logger, mlog_severity_warn, format, args);
	va_end(args);
}

void mlwarnfv(const mlogger_t* logger, mstringview_t format, va_list args)
{
	mllogfv(logger, mlog_severity_warn, format, args);
}

void mldebug(const mlogger_t* logger, mstringview_t message)
{
	mlog_log(logger, mlog_severity_debug, message);
}

void mldebugf(const mlogger_t* logger, mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(logger, mlog_severity_debug, format, args);
	va_end(args);
}

void mldebugfv(const mlogger_t* logger, mstringview_t format, va_list args)
{
	mllogfv(logger, mlog_severity_debug, format, args);
}

void mlinfo(const mlogger_t* logger, mstringview_t message)
{
	mlog_log(logger, mlog_severity_info, message);
}

void mlinfof(const mlogger_t* logger, mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mllogfv(logger, mlog_severity_info, format, args);
	va_end(args);
}

void mlinfofv(const mlogger_t* logger, mstringview_t format, va_list args)
{
	mllogfv(logger, mlog_severity_info, format, args);
}