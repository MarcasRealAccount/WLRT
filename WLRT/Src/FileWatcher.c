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
	uint32_t count;

	uint32_t bufferCap;
	void*    buffer;

	OVERLAPPED overlapped;
	HANDLE     handle;

	bool ready;
} WLRTFileWatcherDirectoryData;

typedef struct WLRTFileWatcherData
{
	bool running;

	uint64_t curId;

	WLRTMutex mutex;

	WLRTDynArray watches;
	WLRTDynArray newDirectories;
	WLRTDynArray cachedDirectories;

	WLRTThreadData watcherThread;
} WLRTFileWatcherData;

static WLRTFileWatcherData s_FileWatcher;

static int WLRTFileWatcherFunc(void* userData)
{
	(void) userData;

	WLRTPath tempPath;
	WLRTPathSetup(&tempPath);

	size_t tempFilenameBufferCap = MAX_PATH * sizeof(wchar_t);
	char*  tempFilenameBuffer    = malloc((tempFilenameBufferCap + 1) * sizeof(char));

	while (s_FileWatcher.running)
	{
		WLRTMutexLock(&s_FileWatcher.mutex);
		WLRTDynArrayReserve(&s_FileWatcher.cachedDirectories, s_FileWatcher.cachedDirectories.size + s_FileWatcher.newDirectories.size);
		for (size_t i = 0; i < s_FileWatcher.newDirectories.size; ++i)
			WLRTDynArrayPushBack(&s_FileWatcher.cachedDirectories, WLRTDynArrayGet(&s_FileWatcher.newDirectories, i));
		WLRTDynArrayClear(&s_FileWatcher.newDirectories);
		WLRTMutexUnlock(&s_FileWatcher.mutex);

		for (size_t i = 0; i < s_FileWatcher.cachedDirectories.size; ++i)
		{
			WLRTFileWatcherDirectoryData* data = (WLRTFileWatcherDirectoryData*) WLRTDynArrayGet(&s_FileWatcher.cachedDirectories, i);
			if (data->count == 0)
			{
				WLRTPathCleanup(&data->directory);
				free(data->buffer);
				CloseHandle(data->overlapped.hEvent);
				CloseHandle(data->handle);
				WLRTDynArrayErase(&s_FileWatcher.cachedDirectories, i);
				--i;
				continue;
			}

			if (data->ready)
			{
				if (!ReadDirectoryChangesW(data->handle, data->buffer, (DWORD) data->bufferCap, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &data->overlapped, NULL))
				{
					// TODO(MarcasRealAccount): Handle failure
					continue;
				}

				data->ready = false;
				continue;
			}

			DWORD res = WaitForSingleObject(data->overlapped.hEvent, 0);
			if (res != WAIT_OBJECT_0)
				continue;

			WLRTPathAssign(&tempPath, &data->directory);

			FILE_NOTIFY_INFORMATION*    pInfo = NULL;
			WLRTFileWatcherCallbackData callbackData;
			memset(&callbackData, 0, sizeof(callbackData));
			do
			{
				if (!pInfo)
					pInfo = (FILE_NOTIFY_INFORMATION*) data->buffer;
				else
					pInfo = (FILE_NOTIFY_INFORMATION*) (((uint8_t*) pInfo) + pInfo->NextEntryOffset);

				switch (pInfo->Action)
				{
				case FILE_ACTION_ADDED:
					callbackData.action = WLRT_FILE_WATCHER_ACTION_CREATE;
					break;
				case FILE_ACTION_REMOVED:
					callbackData.action = WLRT_FILE_WATCHER_ACTION_DELETE;
					break;
				case FILE_ACTION_MODIFIED:
					callbackData.action = WLRT_FILE_WATCHER_ACTION_MODIFY;
					break;
				default:
					continue;
				}

				size_t filenameLen              = WideCharToMultiByte(CP_UTF8, 0, pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t), tempFilenameBuffer, (DWORD) tempFilenameBufferCap, NULL, NULL);
				tempFilenameBuffer[filenameLen] = '\0';

				WLRTPath filename = {
					.path     = tempFilenameBuffer,
					.length   = filenameLen,
					.capacity = tempFilenameBufferCap
				};
				WLRTPathAppend(&tempPath, &filename);
				callbackData.file = &tempPath;

				WLRTMutexLock(&s_FileWatcher.mutex);
				for (size_t j = 0; j < s_FileWatcher.watches.size; ++j)
				{
					WLRTFileWatcherWatchData* watch = (WLRTFileWatcherWatchData*) WLRTDynArrayGet(&s_FileWatcher.watches, j);
					if (WLRTPathCompare(&tempPath, &watch->file) != 0)
						continue;
					callbackData.userData = watch->userData;
					callbackData.id       = watch->id;
					watch->callback(&callbackData);
				}
				WLRTMutexUnlock(&s_FileWatcher.mutex);
			}
			while (pInfo->NextEntryOffset > 0);
			data->ready = true;
		}

		Sleep(250);
	}
	WLRTPathCleanup(&tempPath);

	return 0;
}

bool WLRTFileWatcherSetup()
{
	s_FileWatcher.running                = true;
	s_FileWatcher.curId                  = 0;
	s_FileWatcher.watcherThread.callback = &WLRTFileWatcherFunc;
	s_FileWatcher.watcherThread.userData = NULL;
	if (!WLRTDynArraySetup(&s_FileWatcher.watches, 32, sizeof(WLRTFileWatcherWatchData)) ||
		!WLRTDynArraySetup(&s_FileWatcher.newDirectories, 32, sizeof(WLRTFileWatcherDirectoryData)) ||
		!WLRTDynArraySetup(&s_FileWatcher.cachedDirectories, 32, sizeof(WLRTFileWatcherDirectoryData)) ||
		!WLRTMutexSetup(&s_FileWatcher.mutex) ||
		!WLRTThreadSetup(&s_FileWatcher.watcherThread))
	{
		WLRTFileWatcherCleanup();
		return false;
	}
	WLRTThreadSetName(&s_FileWatcher.watcherThread, "FileWatcher", ~0ULL);
	WLRTThreadStart(&s_FileWatcher.watcherThread);
	return true;
}

void WLRTFileWatcherCleanup()
{
	s_FileWatcher.running = false;
	s_FileWatcher.curId   = 0;
	WLRTDynArrayCleanup(&s_FileWatcher.watches);
	WLRTDynArrayCleanup(&s_FileWatcher.newDirectories);
	WLRTDynArrayCleanup(&s_FileWatcher.cachedDirectories);
	WLRTMutexCleanup(&s_FileWatcher.mutex);
	WLRTThreadCleanup(&s_FileWatcher.watcherThread);
}

uint64_t WLRTFileWatcherWatchFile(const WLRTPath* file, WLRTFileWatcherCallbackFn callback, void* userData)
{
	if (!file || !callback)
		return 0;

	WLRTMutexLock(&s_FileWatcher.mutex);
	++s_FileWatcher.curId;
	{
		WLRTFileWatcherWatchData watch = {
			.id       = s_FileWatcher.curId,
			.file     = WLRTPathCopy(file),
			.callback = callback,
			.userData = userData
		};
		if (!watch.file.path ||
			!WLRTPathAbsolute(&watch.file) ||
			!WLRTDynArrayPushBack(&s_FileWatcher.watches, &watch))
		{
			--s_FileWatcher.curId;
			WLRTPathCleanup(&watch.file);
			WLRTMutexUnlock(&s_FileWatcher.mutex);
			return 0;
		}
	}
	size_t index = s_FileWatcher.watches.size - 1;

	WLRTFileWatcherWatchData* watch = (WLRTFileWatcherWatchData*) WLRTDynArrayGet(&s_FileWatcher.watches, index);

	WLRTPath dir   = WLRTPathGetDirectory(&watch->file);
	bool     found = false;
	for (size_t i = 0; !found && i < s_FileWatcher.newDirectories.size; ++i)
	{
		WLRTFileWatcherDirectoryData* directory = (WLRTFileWatcherDirectoryData*) WLRTDynArrayGet(&s_FileWatcher.newDirectories, i);
		if (WLRTPathCompare(&dir, &directory->directory) == 0)
		{
			++directory->count;
			found = true;
		}
	}
	for (size_t i = 0; !found && i < s_FileWatcher.cachedDirectories.size; ++i)
	{
		WLRTFileWatcherDirectoryData* directory = (WLRTFileWatcherDirectoryData*) WLRTDynArrayGet(&s_FileWatcher.cachedDirectories, i);
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
			.count     = 1,
			.bufferCap = (MAX_PATH * sizeof(wchar_t) + sizeof(FILE_NOTIFY_INFORMATION)) * 8,
			.ready     = true
		};
		memset(&directory.overlapped, 0, sizeof(OVERLAPPED));
		directory.buffer            = malloc(directory.bufferCap);
		directory.overlapped.hEvent = CreateEventA(NULL, FALSE, FALSE, directory.directory.path);
		directory.handle            = CreateFileA(directory.directory.path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (!directory.buffer ||
			!directory.overlapped.hEvent ||
			!directory.handle ||
			!WLRTDynArrayPushBack(&s_FileWatcher.newDirectories, &directory))
		{
			--s_FileWatcher.curId;
			WLRTPathCleanup(&watch->file);
			WLRTDynArrayPopBack(&s_FileWatcher.watches);
			WLRTPathCleanup(&dir);
			free(directory.buffer);
			if (directory.overlapped.hEvent != NULL) // Useless piece of shit
				CloseHandle(directory.overlapped.hEvent);
			if (directory.handle != NULL) // Useless piece of shit
				CloseHandle(directory.handle);
			WLRTMutexUnlock(&s_FileWatcher.mutex);
			return 0;
		}
	}
	WLRTMutexUnlock(&s_FileWatcher.mutex);
	return watch->id;
}

void WLRTFileWatcherUnwatchFile(uint64_t id)
{
	if (id == 0)
		return;

	WLRTMutexLock(&s_FileWatcher.mutex);
	size_t i = 0;
	for (; i < s_FileWatcher.watches.size; ++i)
	{
		WLRTFileWatcherWatchData* watch = (WLRTFileWatcherWatchData*) WLRTDynArrayGet(&s_FileWatcher.watches, i);
		if (watch->id == id)
			break;
	}
	if (i >= s_FileWatcher.watches.size)
		return;

	WLRTFileWatcherWatchData* watch = (WLRTFileWatcherWatchData*) WLRTDynArrayGet(&s_FileWatcher.watches, i);
	WLRTPath                  dir   = WLRTPathGetDirectory(&watch->file);
	bool                      found = false;
	for (size_t j = 0; j < !found && s_FileWatcher.newDirectories.size; ++j)
	{
		WLRTFileWatcherDirectoryData* directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.newDirectories.data) + j;
		if (WLRTPathCompare(&dir, &directory->directory) == 0)
		{
			--directory->count;
			found = true;
		}
	}
	for (size_t j = 0; j < !found && s_FileWatcher.cachedDirectories.size; ++j)
	{
		WLRTFileWatcherDirectoryData* directory = ((WLRTFileWatcherDirectoryData*) s_FileWatcher.cachedDirectories.data) + j;
		if (WLRTPathCompare(&dir, &directory->directory) == 0)
		{
			--directory->count;
			found = true;
		}
	}
	WLRTPathCleanup(&watch->file);
	WLRTDynArrayErase(&s_FileWatcher.watches, i);
	WLRTPathCleanup(&dir);
	WLRTMutexUnlock(&s_FileWatcher.mutex);
}