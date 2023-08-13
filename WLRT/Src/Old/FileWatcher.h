#pragma once

#include "Filesystem.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*FileWatcherCallbackFn)(const FSPath* file, void* userData);

typedef struct FileWatcherWatchData
{
	uint64_t              id;
	FSPath                file;
	FileWatcherCallbackFn callback;
	void*                 userData;
} FileWatcherWatchData;

typedef struct FileWatcherData
{
	uint64_t curId;

	size_t                watchLen;
	size_t                watchCap;
	FileWatcherWatchData* watches;

	size_t  cachedDirectoryLen;
	size_t  cachedDirectoryCap;
	FSPath* cachedDirectories;
} FileWatcherData;

bool     FWSetup();
void     FWCleanup();
void     FWUpdate();
uint64_t FWWatchFile(const FSPath* file, FileWatcherCallbackFn callback, void* userData);
void     FWUnwatchFile(uint64_t id);