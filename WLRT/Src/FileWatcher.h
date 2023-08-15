#pragma once

#include "Filesystem.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum WLRTFileWatcherAction
{
	WLRT_FILE_WATCHER_ACTION_CREATE,
	WLRT_FILE_WATCHER_ACTION_MODIFY,
	WLRT_FILE_WATCHER_ACTION_DELETE,
	WLRT_FILE_WATCHER_ACTION_RENAMED // TODO(MarcasRealAccount): Support renaming
} WLRTFileWatcherAction;

typedef struct WLRTFileWatcherCallbackData
{
	WLRTFileWatcherAction action;
	void*                 userData;
	const WLRTPath*       file;
	const WLRTPath*       newFile;
	uint64_t              id;
} WLRTFileWatcherCallbackData;

typedef void (*WLRTFileWatcherCallbackFn)(const WLRTFileWatcherCallbackData* data);

bool     WLRTFileWatcherSetup();
void     WLRTFileWatcherCleanup();
uint64_t WLRTFileWatcherWatchFile(const WLRTPath* file, WLRTFileWatcherCallbackFn callback, void* userData);
void     WLRTFileWatcherUnwatchFile(uint64_t id);