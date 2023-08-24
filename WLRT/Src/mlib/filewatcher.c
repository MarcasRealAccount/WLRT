#include "filewatcher.h"
#include "atomic.h"
#include "build.h"
#include "dynarray.h"
#include "mutex.h"
#include "thread.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>
#else
#endif

typedef struct mfilewatcher_watch_data_t
{
	uint64_t            id;
	mpath_t             file;
	mfilewatcher_func_t func;
	void*               data;
} mfilewatcher_watch_data_t;

typedef struct mfilewatcher_directory_data_t
{
	mpath_t  directory;
	uint32_t count;

#if BUILD_IS_SYSTEM_WINDOWS
	uint32_t bufferCap;
	void*    buffer;

	OVERLAPPED overlapped;
	HANDLE     handle;

	bool ready;
#else
#endif
} mfilewatcher_directory_data_t;

typedef struct mfilewatcher_t
{
	bool alive;

	mmutex_t mtx;
	uint64_t curId;

	mdynarray_t watches;
	mdynarray_t dirWatches;
	mdynarray_t newDirs;
	mdynarray_t cachedDirs;

	mthread_t thread;
} mfilewatcher_t;

static mfilewatcher_t s_FileWatcher;

static uint64_t mfilewatcher_func(void* pData)
{
	(void) pData;

	mpath_t tempPath1 = mpath(0);
	mpath_t tempPath2 = mpath(0);

#if BUILD_IS_SYSTEM_WINDOW
	char buffer[MAX_PATH * sizeof(wchar_t)];
#else
#endif

	while (matomicbool_load(&s_FileWatcher.alive))
	{
		if (s_FileWatcher.newDirs.size > 0)
		{
			mmutex_lock(&s_FileWatcher.mtx);
			mdynarray_reserve(&s_FileWatcher.cachedDirs, s_FileWatcher.cachedDirs.size + s_FileWatcher.newDirs.size);
			for (size_t i = 0; i < s_FileWatcher.newDirs.size; ++i)
				mdynarray_pushback(&s_FileWatcher.cachedDirs, mdynarray_get(&s_FileWatcher.newDirs, i));
			mdynarray_clear(&s_FileWatcher.newDirs);
			mmutex_unlock(&s_FileWatcher.mtx);
		}

		for (size_t i = 0; i < s_FileWatcher.cachedDirs.size; ++i)
		{
			mfilewatcher_directory_data_t* directory = (mfilewatcher_directory_data_t*) mdynarray_get(&s_FileWatcher.cachedDirs, i);
			if (directory->count == 0)
			{
#if BUILD_IS_SYSTEM_WINDOWS
				mfree(directory->buffer);
				CloseHandle(directory->overlapped.hEvent);
				CloseHandle(directory->handle);
#else
#endif
				mpath_dstr(&directory->directory);
				mdynarray_erase(&s_FileWatcher.cachedDirs, i, 1);
				--i;
				continue;
			}

#if BUILD_IS_SYSTEM_WINDOWS
			if (directory->ready)
			{
				if (!ReadDirectoryChangesW(directory->handle, directory->buffer, directory->bufferCap, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &directory->overlapped, NULL))
				{
					// TODO(MarcasRealAccount): Handle failure in mfilewatcher_func
					continue;
				}

				directory->ready = false;
				continue;
			}

			DWORD res = WaitForSingleObject(directory->overlapped.hEvent, 0);
			if (res != WAIT_OBJECT_0)
				continue;

			mpath_assign(&tempPath1, mstringviewp(&directory->directory));
			mpath_assign(&tempPath2, mstringviewp(&directory->directory));

			FILE_NOTIFY_INFORMATION* pInfo = NULL;
			mfilewatcher_data_t      data;
			memset(&data, 0, sizeof(data));
			do {
				if (!pInfo)
					pInfo = (FILE_NOTIFY_INFORMATION*) directory->buffer;
				else
					pInfo = (FILE_NOTIFY_INFORMATION*) (((uint8_t*) pInfo) + pInfo->NextEntryOffset);

				switch (pInfo->Action)
				{
				case FILE_ACTION_ADDED:
					data.action = mfilewatcher_action_create;
					break;
				case FILE_ACTION_REMOVED:
					data.action = mfilewatcher_action_delete;
					break;
				case FILE_ACTION_MODIFIED:
					data.action = mfilewatcher_action_modify;
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					data.action = mfilewatcher_action_rename;
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					break;
				default:
					continue;
				}

				int len = WideCharToMultiByte(CP_UTF8, 0, pInfo->FileName, (int) pInfo->FileNameLength / sizeof(wchar_t), buffer, (int) sizeof(buffer), NULL, NULL);

				if (pInfo->Action == FILE_ACTION_RENAMED_NEW_NAME)
				{
					mpath_append(&tempPath2, mstringview(buffer, len));
					data.newFile = &tempPath2;
				}
				else
				{
					mpath_append(&tempPath1, mstringview(buffer, len));
					data.file = &tempPath1;
				}
				if (pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME)
					continue;

				mmutex_lock(&s_FileWatcher.mtx);
				for (size_t j = 0; j < s_FileWatcher.watches.size; ++j)
				{
					mfilewatcher_watch_data_t* watch = (mfilewatcher_watch_data_t*) mdynarray_get(&s_FileWatcher.watches, j);
					if (data.action == mfilewatcher_action_rename)
					{
						if (mpath_compare(&tempPath1, &watch->file) != 0 &&
							mpath_compare(&tempPath2, &watch->file) != 0)
							continue;
					}
					else if (mpath_compare(&tempPath1, &watch->file) != 0)
						continue;
					data.data = watch->data;
					data.id   = watch->id;
					watch->func(&data);
				}
				mmutex_unlock(&s_FileWatcher.mtx);
			}
			while (true);
			directory->ready = true;
#else
#endif
		}

		mthread_sleep(250);
	}

	mpath_dstr(&tempPath1);
	mpath_dstr(&tempPath2);
	return 0;
}

bool mfilewatcher_init()
{
	matomicbool_store(&s_FileWatcher.alive, true);
	mmutex_cstr(&s_FileWatcher.mtx);
	s_FileWatcher.curId = 0;
	mdynarray_cstr(&s_FileWatcher.watches, 32, sizeof(mfilewatcher_watch_data_t));
	mdynarray_cstr(&s_FileWatcher.dirWatches, 32, sizeof(mfilewatcher_watch_data_t));
	mdynarray_cstr(&s_FileWatcher.newDirs, 32, sizeof(mfilewatcher_directory_data_t));
	mdynarray_cstr(&s_FileWatcher.cachedDirs, 32, sizeof(mfilewatcher_directory_data_t));
	mthread_cstr(&s_FileWatcher.thread, &mfilewatcher_func, NULL);
	mthread_set_name(&s_FileWatcher.thread, mstringviewcstr("mfilewatcher"));
	mthread_start(&s_FileWatcher.thread);
	return true;
}

void mfilewatcher_deinit()
{
	matomicbool_store(&s_FileWatcher.alive, false);
	s_FileWatcher.curId = 0;
	mthread_dstr(&s_FileWatcher.thread);
	mdynarray_dstr(&s_FileWatcher.watches);
	mdynarray_dstr(&s_FileWatcher.dirWatches);
	mdynarray_dstr(&s_FileWatcher.newDirs);
	mdynarray_dstr(&s_FileWatcher.cachedDirs);
	mmutex_dstr(&s_FileWatcher.mtx);
}

uint64_t mfilewatcher_watch(const mpath_t* file, mfilewatcher_func_t func, void* data)
{
	if (!file || !func)
		return 0;

	mmutex_lock(&s_FileWatcher.mtx);
	++s_FileWatcher.curId;
	{
		mfilewatcher_watch_data_t watch = {
			.id   = s_FileWatcher.curId,
			.file = mpathv(mstringviewp(file)),
			.func = func,
			.data = data
		};
		mpath_weakly_canonical(&watch.file);
		mdynarray_pushback(&s_FileWatcher.watches, &watch);
	}
	size_t index = s_FileWatcher.watches.size - 1;

	mfilewatcher_watch_data_t* watch = (mfilewatcher_watch_data_t*) mdynarray_get(&s_FileWatcher.watches, index);

	mstringview_t dir   = mpath_dir(&watch->file);
	bool          found = false;
	for (size_t i = 0; !found && i < s_FileWatcher.newDirs.size; ++i)
	{
		mfilewatcher_directory_data_t* directory = (mfilewatcher_directory_data_t*) mdynarray_get(&s_FileWatcher.newDirs, i);
		if (mstringview_compare(dir, mstringviewp(&directory->directory)))
		{
			++directory->count;
			found = true;
		}
	}
	for (size_t i = 0; !found && i < s_FileWatcher.cachedDirs.size; ++i)
	{
		mfilewatcher_directory_data_t* directory = (mfilewatcher_directory_data_t*) mdynarray_get(&s_FileWatcher.cachedDirs, i);
		if (mstringview_compare(dir, mstringviewp(&directory->directory)))
		{
			++directory->count;
			found = true;
		}
	}
	if (!found)
	{
		mfilewatcher_directory_data_t directory = {
			.directory = mpathv(dir),
			.count     = 1
		};
#if BUILD_IS_SYSTEM_WINDOWS
		directory.bufferCap = (MAX_PATH * sizeof(wchar_t) + sizeof(FILE_NOTIFY_INFORMATION)) * 8;
		directory.buffer    = mmalloc(directory.bufferCap);
		memset(&directory.overlapped, 0, sizeof(OVERLAPPED));
		wchar_t wbuffer[32768];
		int     newLen              = MultiByteToWideChar(CP_UTF8, 0, mpath_begin(&directory.directory), (int) directory.directory.length, wbuffer, 32767);
		wbuffer[newLen]             = L'\0';
		directory.overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, wbuffer);
		directory.handle            = CreateFileW(wbuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
#else
#endif
		mdynarray_pushback(&s_FileWatcher.newDirs, &directory);
	}
	uint64_t id = watch->id;
	mmutex_unlock(&s_FileWatcher.mtx);
	return id;
}

void mfilewatcher_unwatch(uint64_t id)
{
	if (id == 0)
		return;

	mmutex_lock(&s_FileWatcher.mtx);
	size_t i = 0;
	for (; i < s_FileWatcher.watches.size; ++i)
	{
		mfilewatcher_watch_data_t* watch = (mfilewatcher_watch_data_t*) mdynarray_get(&s_FileWatcher.watches, i);
		if (watch->id == id)
			break;
	}
	mfilewatcher_watch_data_t* watch = (mfilewatcher_watch_data_t*) mdynarray_get(&s_FileWatcher.watches, i);
	mstringview_t              dir   = mpath_dir(&watch->file);
	bool                       found = false;
	for (size_t j = 0; !found || j < s_FileWatcher.newDirs.size; ++j)
	{
		mfilewatcher_directory_data_t* directory = (mfilewatcher_directory_data_t*) mdynarray_get(&s_FileWatcher.newDirs, i);
		if (mstringview_compare(dir, mstringviewp(&directory->directory)))
		{
			--directory->count;
			found = true;
		}
	}
	for (size_t j = 0; !found || j < s_FileWatcher.cachedDirs.size; ++j)
	{
		mfilewatcher_directory_data_t* directory = (mfilewatcher_directory_data_t*) mdynarray_get(&s_FileWatcher.cachedDirs, i);
		if (mstringview_compare(dir, mstringviewp(&directory->directory)))
		{
			--directory->count;
			found = true;
		}
	}
	mpath_dstr(&watch->file);
	mdynarray_erase(&s_FileWatcher.watches, i, 1);
	mmutex_unlock(&s_FileWatcher.mtx);
}
