#include "FileWatcher.h"

#include <stdlib.h>
#include <string.h>

#include <Windows.h>

static FileWatcherData* s_FileWatcher;

bool FWSetup()
{
	s_FileWatcher = (FileWatcherData*) malloc(sizeof(FileWatcherData));
	if (!s_FileWatcher)
		return false;
	s_FileWatcher->watchLen = 0;
	s_FileWatcher->watchCap = 32;
	s_FileWatcher->watches  = (FileWatcherWatchData*) malloc(32 * sizeof(FileWatcherWatchData));
	if (!s_FileWatcher->watches)
	{
		FWCleanup();
		return false;
	}

	s_FileWatcher->cachedDirectoryLen = 0;
	s_FileWatcher->cachedDirectoryCap = 32;
	s_FileWatcher->cachedDirectories  = (FSPath*) malloc(32 * sizeof(FSPath));
	if (!s_FileWatcher->cachedDirectories)
	{
		FWCleanup();
		return false;
	}
	memset(s_FileWatcher->watches, 0, s_FileWatcher->watchCap * sizeof(FileWatcherWatchData));
	memset(s_FileWatcher->cachedDirectories, 0, s_FileWatcher->cachedDirectoryCap * sizeof(FSPath));
	s_FileWatcher->curId = 0;
	return true;
}

void FWCleanup()
{
	if (!s_FileWatcher)
		return;

	for (size_t i = 0; i < s_FileWatcher->watchLen; ++i)
		FSDestroyPath(&s_FileWatcher->watches[i].file);
	free(s_FileWatcher->watches);
	for (size_t i = 0; i < s_FileWatcher->cachedDirectoryLen; ++i)
		FSDestroyPath(s_FileWatcher->cachedDirectories + i);
	free(s_FileWatcher->cachedDirectories);
	free(s_FileWatcher);
	s_FileWatcher = NULL;
}

void FWUpdate()
{
	if (!s_FileWatcher)
		return;

	uint8_t* buffer = (uint8_t*) malloc(32768 * sizeof(uint8_t));
	if (!buffer)
		return;

	FSPath tempPath = {
		.buf = (char*) malloc(32768 * sizeof(char)),
		.cap = 32768,
		.len = 0
	};
	for (size_t i = 0; i < s_FileWatcher->cachedDirectoryLen; ++i)
	{
		FSPath* path    = s_FileWatcher->cachedDirectories + i;
		HANDLE  dHandle = CreateFileA(path->buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		DWORD   written = 0;
		bool    res     = ReadDirectoryChangesW(dHandle, buffer, 32768 * sizeof(uint8_t), false, FILE_NOTIFY_CHANGE_LAST_WRITE, &written, NULL, NULL);
		CloseHandle(dHandle);
		if (!res)
			continue;

		size_t                   offset = 0;
		FILE_NOTIFY_INFORMATION* info;
		do {
			info = (FILE_NOTIFY_INFORMATION*) (buffer + offset);
			switch (info->Action)
			{
			case FILE_ACTION_MODIFIED:
			{
				tempPath.len = wcstombs(tempPath.buf, info->FileName, tempPath.cap);
				for (size_t j = 0; j < s_FileWatcher->watchLen; ++j)
				{
					FileWatcherWatchData* watch = s_FileWatcher->watches + j;
					if (FSPathEquals(&tempPath, &watch->file))
						watch->callback(&watch->file, watch->userData);
				}
				break;
			}
			}
			offset += info->NextEntryOffset;
		}
		while (info->NextEntryOffset);
	}
	FSDestroyPath(&tempPath);
	free(buffer);
}

static bool FWEnsureWatchCap(size_t requiredSize)
{
	if (!s_FileWatcher)
		return false;

	if (requiredSize <= s_FileWatcher->watchCap)
		return true;

	size_t newCapacity = requiredSize;
	newCapacity       |= newCapacity >> 1;
	newCapacity       |= newCapacity >> 2;
	newCapacity       |= newCapacity >> 4;
	newCapacity       |= newCapacity >> 8;
	newCapacity       |= newCapacity >> 16;
	newCapacity       |= newCapacity >> 32;
	++newCapacity;

	FileWatcherWatchData* newWatches = (FileWatcherWatchData*) malloc(newCapacity * sizeof(FileWatcherWatchData));
	if (!newWatches)
		return false;
	memcpy(newWatches, s_FileWatcher->watches, s_FileWatcher->watchCap);
	memset(newWatches + s_FileWatcher->watchCap, 0, (newCapacity - s_FileWatcher->watchCap) * sizeof(FileWatcherWatchData));
	free(s_FileWatcher->watches);
	s_FileWatcher->watchCap = newCapacity;
	s_FileWatcher->watches  = newWatches;
	return true;
}

static bool FWEnsureCachedDirectoryCap(size_t requiredSize)
{
	if (!s_FileWatcher)
		return false;

	if (requiredSize <= s_FileWatcher->cachedDirectoryCap)
		return true;

	size_t newCapacity = requiredSize;
	newCapacity       |= newCapacity >> 1;
	newCapacity       |= newCapacity >> 2;
	newCapacity       |= newCapacity >> 4;
	newCapacity       |= newCapacity >> 8;
	newCapacity       |= newCapacity >> 16;
	newCapacity       |= newCapacity >> 32;
	++newCapacity;

	FSPath* newCachedDirectories = (FSPath*) malloc(newCapacity * sizeof(FSPath));
	if (!newCachedDirectories)
		return false;
	memcpy(newCachedDirectories, s_FileWatcher->cachedDirectories, s_FileWatcher->cachedDirectoryCap);
	memset(newCachedDirectories + s_FileWatcher->cachedDirectoryCap, 0, (newCapacity - s_FileWatcher->cachedDirectoryCap) * sizeof(FSPath));
	free(s_FileWatcher->cachedDirectories);
	s_FileWatcher->cachedDirectoryCap = newCapacity;
	s_FileWatcher->cachedDirectories  = newCachedDirectories;
	return true;
}

uint64_t FWWatchFile(const FSPath* file, FileWatcherCallbackFn callback, void* userData)
{
	if (!s_FileWatcher ||
		!FWEnsureWatchCap(s_FileWatcher->watchLen + 1))
		return 0;

	FileWatcherWatchData* watch = s_FileWatcher->watches + s_FileWatcher->watchLen;
	++s_FileWatcher->watchLen;
	watch->id       = ++s_FileWatcher->curId;
	watch->file     = FSCreatePath(file->buf, file->len);
	watch->callback = callback;
	watch->userData = userData;

	FSPath stem  = FSPathGetStem(file);
	bool   found = false;
	for (size_t i = 0; i < s_FileWatcher->cachedDirectoryLen; ++i)
	{
		FSPath* path = s_FileWatcher->cachedDirectories + i;
		if (FSPathEquals(&stem, path))
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		do {
			if (!FWEnsureCachedDirectoryCap(s_FileWatcher->cachedDirectoryLen + 1))
				break;

			FSPath* path = s_FileWatcher->cachedDirectories + s_FileWatcher->cachedDirectoryLen;
			++s_FileWatcher->cachedDirectoryLen;
			*path = FSCreatePath(stem.buf, stem.len);
		}
		while (false);
	}
	FSDestroyPath(&stem);
	return watch->id;
}

void FWUnwatchFile(uint64_t id)
{
	if (!s_FileWatcher)
		return;

	size_t i = 0;
	for (; i < s_FileWatcher->watchLen; ++i)
	{
		FileWatcherWatchData* watch = s_FileWatcher->watches + i;
		if (watch->id == id)
			break;
	}
	if (i >= s_FileWatcher->watchLen)
		return;
	FileWatcherWatchData* foundWatch = s_FileWatcher->watches + i;

	FSPath stem = FSPathGetStem(&foundWatch->file);
	FSDestroyPath(&foundWatch->file);
	memmove(s_FileWatcher->watches + i - 1, s_FileWatcher->watches + i, (s_FileWatcher->watchLen - i) * sizeof(FileWatcherWatchData));
	--s_FileWatcher->watchLen;
	memset(s_FileWatcher->watches + s_FileWatcher->watchLen, 0, sizeof(FileWatcherWatchData));

	bool found = false;
	for (i = 0; i < s_FileWatcher->watchLen; ++i)
	{
		FileWatcherWatchData* watch = s_FileWatcher->watches + i;
		if (watch == foundWatch)
			continue;

		FSPath stem2 = FSPathGetStem(&watch->file);
		if (FSPathEquals(&stem, &stem2))
		{
			FSDestroyPath(&stem2);
			found = true;
			break;
		}
		FSDestroyPath(&stem2);
	}
	if (!found)
	{
		i = 0;
		for (; i < s_FileWatcher->cachedDirectoryLen; ++i)
		{
			if (FSPathEquals(&stem, s_FileWatcher->cachedDirectories + i))
				break;
		}
		memmove(s_FileWatcher->cachedDirectories + i - 1, s_FileWatcher->cachedDirectories + i, (s_FileWatcher->cachedDirectoryLen - i) * sizeof(FSPath));
		--s_FileWatcher->cachedDirectoryLen;
		memset(s_FileWatcher->cachedDirectories + s_FileWatcher->cachedDirectoryLen, 0, sizeof(FSPath));
	}
	FSDestroyPath(&stem);
}