#include "FileWatcher.h"
#include "DynArray.h"
#include "Threading.h"

#include <stdlib.h>

#include <Windows.h>

typedef struct WLRTFileWatcherWatchData
{
	uint64_t                  id;
	WLRTPath                  file;
	WLRTFileWatcherCallbackFn callback;
	void*                     userData;
} WLRTFileWatcherWatchData;

typedef struct WLRTFileWatcherDirectoryData
{
	WLRTPath directory;
	uint64_t count;
	HANDLE   handle;
} WLRTFileWatcherDirectoryData;

typedef struct WLRTFileWatcherData
{
	uint64_t curId;

	WLRTDynArray watches;
	WLRTDynArray newDirectories;
	WLRTDynArray cachedDirectories;

	WLRTThreadData watcherThread;

	OVERLAPPED overlapped;
	size_t     bufferCap;
	void*      buffer;
} WLRTFileWatcherData;

static WLRTFileWatcherData s_FileWatcher;

static void WLRTFileWatcherDirChangeNotify(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)
{
}

static int WLRTFileWatcherFunc(void* userData)
{
	(void) userData;


	for (size_t i = 0; i < s_FileWatcher.cachedDirectories.size; ++i)
	{
		WLRTFileWatcherDirectoryData* directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.cachedDirectories.data) + i;
		if (directory->count == 0)
		{
			WLRTDynArrayErase(&s_FileWatcher.cachedDirectories, i);
			--i;
			directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.cachedDirectories.data) + i;
			CloseHandle(directory->handle);
		}
		if (!directory->handle)
		{
			directory->handle = CreateFileA(directory->directory.path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		}
		if (!directory->handle)
			continue;

		ReadDirectoryChangesW(directory->handle, s_FileWatcher.buffer, s_FileWatcher.bufferCap, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &s_FileWatcher.overlapped, &WLRTFileWatcherDirChangeNotify);
	}
	return 0;
}

bool WLRTFileWatcherSetup()
{
	memset(&s_FileWatcher.overlapped, 0, sizeof(OVERLAPPED));
	s_FileWatcher.curId                  = 0;
	s_FileWatcher.watcherThread.callback = &WLRTFileWatcherFunc;
	s_FileWatcher.watcherThread.userData = NULL;
	if (!WLRTDynArraySetup(&s_FileWatcher.watches, 32, sizeof(WLRTFileWatcherWatchData)) ||
		!WLRTDynArraySetup(&s_FileWatcher.newDirectories, 32, sizeof(WLRTFileWatcherDirectoryData)) ||
		!WLRTDynArraySetup(&s_FileWatcher.cachedDirectories, 32, sizeof(WLRTFileWatcherDirectoryData)) ||
		!WLRTThreadSetup(&s_FileWatcher.watcherThread))
	{
		WLRTFileWatcherCleanup();
		return false;
	}
	s_FileWatcher.bufferCap = 32768;
	s_FileWatcher.buffer    = malloc(s_FileWatcher.bufferCap);
	if (!s_FileWatcher.buffer)
	{
		WLRTFileWatcherCleanup();
		return false;
	}
	WLRTThreadSetName(&s_FileWatcher.watcherThread, "FileWatcher", ~0ULL);
	return true;
}

void WLRTFileWatcherCleanup()
{
	s_FileWatcher.curId = 0;
	WLRTDynArrayCleanup(&s_FileWatcher.watches);
	WLRTDynArrayCleanup(&s_FileWatcher.newDirectories);
	WLRTDynArrayCleanup(&s_FileWatcher.cachedDirectories);
	WLRTThreadCleanup(&s_FileWatcher.watcherThread);
	free(s_FileWatcher.buffer);
	s_FileWatcher.bufferCap = 0;
	s_FileWatcher.buffer    = NULL;
}

uint64_t WLRTFileWatcherWatchFile(const WLRTPath* file, WLRTFileWatcherCallbackFn callback, void* userData)
{
	if (!file || !callback)
		return 0;

	WLRTFileWatcherWatchData watch = {
		.id       = s_FileWatcher.curId + 1,
		.file     = WLRTPathCopy(file),
		.callback = callback,
		.userData = userData
	};
	if (!WLRTDynArrayPushBack(&s_FileWatcher.watches, &watch))
		return 0;
	++s_FileWatcher.curId;

	WLRTPath dir   = WLRTPathGetDirectory(&((WLRTFileWatcherWatchData*) s_FileWatcher.watches.data)[s_FileWatcher.watches.size - 1].file);
	bool     found = false;
	for (size_t i = 0; !found && i < s_FileWatcher.newDirectories.size; ++i)
	{
		WLRTFileWatcherDirectoryData* directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.newDirectories.data) + i;
		if (WLRTPathCompare(&dir, &directory->directory) == 0)
		{
			++directory->count;
			found = true;
		}
	}
	for (size_t i = 0; !found && i < s_FileWatcher.cachedDirectories.size; ++i)
	{
		WLRTFileWatcherDirectoryData* directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.cachedDirectories.data) + i;
		if (WLRTPathCompare(&dir, &directory->directory) == 0)
		{
			++directory->count;
			found = true;
		}
	}
	if (!found)
	{
		WLRTFileWatcherDirectoryData directory = {
			.directory = dir,
			.count     = 1
		};
		WLRTDynArrayPushBack(&s_FileWatcher.newDirectories, &directory);
	}
	return watch.id;
}

void WLRTFileWatcherUnwatchFile(uint64_t id)
{
	if (id == 0)
		return;

	size_t i = 0;
	for (; i < s_FileWatcher.watches.size; ++i)
	{
		WLRTFileWatcherWatchData* watch = ((WLRTFileWatcherWatchData*) s_FileWatcher.watches.data) + i;
		if (watch->id == id)
			break;
	}
	if (i >= s_FileWatcher.watches.size)
		return;

	WLRTFileWatcherWatchData* watch = ((WLRTFileWatcherWatchData*) s_FileWatcher.watches.data) + i;
	WLRTPath                  dir   = WLRTPathGetDirectory(&watch->file);
	bool                      found = false;
	for (size_t j = 0; j < !found && s_FileWatcher.newDirectories.size; ++j)
	{
		WLRTFileWatcherDirectoryData* directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.newDirectories.data) + j;
		if (WLRTPathCompare(&dir, &directory->directory) == 0)
		{
			--directory->count == 0;
			found = true;
		}
	}
	for (size_t j = 0; j < !found && s_FileWatcher.cachedDirectories.size; ++j)
	{
		WLRTFileWatcherDirectoryData* directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.cachedDirectories.data) + j;
		if (WLRTPathCompare(&dir, &directory->directory) == 0)
		{
			--directory->count == 0;
			found = true;
		}
	}
}