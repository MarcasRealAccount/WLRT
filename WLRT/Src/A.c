#include "mlib/atomic.h"
#include "mlib/init.h"
#include "mlib/log.h"
#include "mlib/thread.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <intrin.h>

typedef struct data_t
{
	size_t* b;

	bool first;

	size_t worstLatency;
	size_t bestLatency;
	double avgLatency;

	bool alive;
} data_t;

uint64_t func(void* pData)
{
	data_t* data = (data_t*) pData;
	while (matomicbool_load(&data->alive))
	{
		size_t a = __rdtsc();
		matomic64_wait(data->b, matomic64_load(data->b));
		size_t b   = __rdtsc();
		size_t lat = b - a;
		if (lat > data->worstLatency)
			data->worstLatency = lat;
		if (lat < data->bestLatency)
			data->bestLatency = lat;
		data->avgLatency += lat;
		mwarnf(mstringviewcstr("Funky stuff %s"), "lmao");
	}
	return 0;
}

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

	size_t iters = 0;

	data_t data1, data2;
	memset(&data1, 0, sizeof(data1));
	memset(&data2, 0, sizeof(data2));
	data1.b            = &iters;
	data2.b            = &iters;
	data1.first        = true;
	data1.worstLatency = 0;
	data1.bestLatency  = ~0ULL;
	data1.alive        = true;
	data2.worstLatency = 0;
	data2.bestLatency  = ~0ULL;
	data2.alive        = true;
	mthread_t thread1, thread2;
	mthread_cstr(&thread1, &func, &data1);
	mthread_cstr(&thread2, &func, &data2);
	mthread_set_name(&thread1, mstringviewcstr("Thread1"));
	mthread_set_name(&thread2, mstringviewcstr("Thread2"));

	mthread_start(&thread1);
	mthread_start(&thread2);
	for (size_t i = 0; i < 10'000'000; ++i)
	{
		matomic64_fetch_add(&iters, 1);
		matomic64_notify_all(&iters);
	}
	matomicbool_store(&data1.alive, false);
	matomicbool_store(&data2.alive, false);
	mthread_dstr(&thread1);
	mthread_dstr(&thread2);

	mwarnf(mstringviewcstr("Iters: %u, Worst: %u, Best: %u, Avg: %u"), iters, data1.worstLatency, data1.bestLatency, (uint64_t) (data1.avgLatency / iters));
	merrorf(mstringviewcstr("Iters: %u, Worst: %u, Best: %u, Avg: %u"), iters, data2.worstLatency, data2.bestLatency, (uint64_t) (data2.avgLatency / iters));

	mlib_deinit();
	return 0;
}