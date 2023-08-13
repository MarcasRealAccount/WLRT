#pragma once

#include "Filesystem.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*WLRTFileWatcherCallbackFn)(const WLRTPath* file, void* userData);

bool     WLRTFileWatcherSetup();
void     WLRTFileWatcherCleanup();
uint64_t WLRTFileWatcherWatchFile(const WLRTPath* file, WLRTFileWatcherCallbackFn callback, void* userData);
void     WLRTFileWatcherUnwatchFile(uint64_t id);