#include "init.h"
#include "exit.h"
#include "filewatcher.h"
#include "log.h"
#include "profiler.h"
#include "thread.h"
#include "tim.h"

static void mprofiler_on_exit(void* data)
{
	(void) data;
	mprofiler_deinit();
}

static void mtime_on_exit(void* data)
{
	(void) data;
	mtime_deinit();
}

static void mthread_on_exit(void* data)
{
	(void) data;
	mthread_deinit();
}

static void mlog_on_exit(void* data)
{
	(void) data;
	mlog_deinit();
}

static void mfilewatcher_on_exit(void* data)
{
	(void) data;
	mfilewatcher_deinit();
}

bool mlib_init()
{
	if (!mexit_init())
		return false;
	massert_register(mprofiler_init(), 1, &mprofiler_on_exit, NULL);
	massert_register(mtime_init(), 1, &mtime_on_exit, NULL);
	massert_register(mthread_init(), 1, &mthread_on_exit, NULL);
	massert_register(mlog_init(), 1, &mlog_on_exit, NULL);
	massert_register(mfilewatcher_init(), 1, &mfilewatcher_on_exit, NULL);
	return true;
}

void mlib_deinit()
{
	mexit_deinit();
}