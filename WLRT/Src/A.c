#include "mlib/atomic.h"
#include "mlib/init.h"
#include "mlib/log.h"
#include "mlib/thread.h"
#include "mlib/tim.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static void consoleWrite(void* data, bool isError, mstringview_t buffer)
{
	(void) data;
	fwrite(buffer.string, sizeof(char), buffer.length, isError ? stderr : stdout);
}

static void consoleFlush(void* data, bool isError)
{
	(void) data;
	fflush(isError ? stderr : stdout);
}

int main()
{
	if (!mlib_init())
		return 1;

	mlog_sink_t consoleSink = {
		.severity = mlog_severity_info,
		.write    = &consoleWrite,
		.flush    = &consoleFlush,
		.data     = NULL
	};
	mlog_sink_register(&consoleSink);

	size_t   iterCount = 100000ULL;
	uint64_t start     = mtime_high_res();
	for (size_t i = 0; i < iterCount; ++i)
		merrorf(mstringviewcstr("Test message %u"), i);
	uint64_t end = mtime_high_res();

	uint64_t ns    = (uint64_t) ((end - start) * mtime_high_res_factor());
	uint64_t avgNs = (uint64_t) ((end - start) * mtime_high_res_factor() / iterCount);
	minfof(mstringviewcstr("merrorf total time %u, avg time %u"), ns, avgNs);

	mlib_deinit();
	return 0;
}