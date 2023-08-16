#include "Exit.h"
#include "FileWatcher.h"
#include "Logging.h"
#include "Window.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void WLRTPrintLogSink(void* userdata, bool isError, WLRTStringView buffer)
{
	(void) userdata;
	fwrite(buffer.string, sizeof(char), buffer.length, isError ? stderr : stdout);
}

static void WLRTPrintLogSinkFlush(void* userdata, bool isError)
{
	(void) userdata;
	fflush(isError ? stderr : stdout);
}

static void WLRTLogOnExit(void* userdata)
{
	(void) userdata;
	WLRTLogCleanup();
}

static void WLRTWindowingOnExit(void* userdata)
{
	(void) userdata;
	WLRTWindowingCleanup();
}

static void WLRTWindowOnExit(void* userdata)
{
	WLRTWindowCleanup((WLRTWindowData*) userdata);
}

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	if (!WLRTExitSetup())
		return 1;

	WLRTExitAssert(WLRTLogSetup(), 1);
	WLRTExitRegister(&WLRTLogOnExit, NULL);

	WLRTLogSinkData consoleSink = {
		.severity = WLRT_LOG_SEVERITY_INFO,
		.writer   = &WLRTPrintLogSink,
		.flusher  = &WLRTPrintLogSinkFlush,
		.userdata = NULL
	};
	WLRTSinkRegister(&consoleSink);

	WLRTExitAssert(WLRTWindowingSetup(), 1);
	WLRTExitRegister(&WLRTWindowingOnExit, NULL);

	WLRTWindowData window = {
		.x      = 1 << 31,
		.y      = 1 << 31,
		.width  = 1280,
		.height = 720
	};
	WLRTExitAssert(WLRTWindowSetup(&window), 1);
	WLRTExitRegister(&WLRTWindowOnExit, &window);
	WLRTWindowSetVisible(&window, true);

	while (!window.wantsClose)
	{
		WLRTWindowingPollEvents();
	}

	WLRTExitHandle();
	return 0;
}