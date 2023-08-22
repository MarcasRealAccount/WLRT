#include "thread.h"
#include "atomic.h"
#include "build.h"
#include "dynarray.h"
#include "exit.h"
#include "mutex.h"

#include <setjmp.h>

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>
#else
#endif

typedef struct mthread_storage_t
{
	mthread_t*  thread;
	mthreadid_t id;

	volatile void* waitAddress;
	const void*    waitExpected;
	size_t         waitExpectedSize;

#if BUILD_IS_SYSTEM_WINDOWS
	HANDLE waitEvent;
#else
#endif

	jmp_buf returnJmpBuf;
} mthread_storage_t;

static size_t          s_ThreadStorageID = ~0ULL;
static mshared_mutex_t s_ThreadsMtx;
static mdynarray_t     s_Threads;

static DWORD mthread_thunk(PVOID data)
{
	mthread_t* thread = (mthread_t*) data;

	mthread_storage_t* storage = (mthread_storage_t*) mmalloc(sizeof(mthread_storage_t));
	storage->thread            = thread;
	storage->id                = thread->id;
	storage->waitAddress       = NULL;
	storage->waitExpected      = NULL;
	storage->waitExpectedSize  = 0;
#if BUILD_IS_SYSTEM_WINDOWS
	storage->waitEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
#else
#endif
	mthread_storage_set(s_ThreadStorageID, storage);
	mshared_mutex_lock_busy(&s_ThreadsMtx);
	mdynarray_pushback(&s_Threads, &storage);
	mshared_mutex_unlock_busy(&s_ThreadsMtx);

	if (!setjmp(storage->returnJmpBuf))
	{
		thread->exitCode = thread->func(thread->data);
		mexit_handle();
	}

	mshared_mutex_lock_busy(&s_ThreadsMtx);
	for (size_t i = 0; i < s_Threads.size; ++i)
	{
		if (*(mthread_storage_t**) mdynarray_get(&s_Threads, i) != storage)
			continue;
		mdynarray_erase(&s_Threads, i, 1);
		break;
	}
	mshared_mutex_unlock_busy(&s_ThreadsMtx);
#if BUILD_IS_SYSTEM_WINDOWS
	if (storage->waitEvent)
		CloseHandle(storage->waitEvent);
#else
#endif
	mthread_storage_set(s_ThreadStorageID, NULL);
	mfree(storage);
	return 0;
}

bool mthread_init()
{
	s_ThreadStorageID = mthread_storage_alloc();
	mshared_mutex_cstr(&s_ThreadsMtx);
	mdynarray_cstr(&s_Threads, 32, sizeof(mthread_storage_t*));

#if BUILD_IS_SYSTEM_WINDOWS
	DWORD tid = GetCurrentThreadId();
#else
#endif

	mthread_storage_t* storage = (mthread_storage_t*) mmalloc(sizeof(mthread_storage_t));
	storage->thread            = NULL;
	storage->id                = tid;
	storage->waitAddress       = NULL;
	storage->waitExpected      = NULL;
	storage->waitExpectedSize  = 0;
#if BUILD_IS_SYSTEM_WINDOWS
	storage->waitEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
#else
#endif
	mthread_storage_set(s_ThreadStorageID, storage);
	mdynarray_pushback(&s_Threads, &storage);
	return true;
}

void mthread_deinit()
{
	for (size_t i = 0; i < s_Threads.size; ++i)
	{
		mthread_storage_t* storage = *(mthread_storage_t**) mdynarray_get(&s_Threads, i);
		mthread_dstr(storage->thread);
#if BUILD_IS_SYSTEM_WINDOWS
		CloseHandle(storage->waitEvent);
#else
#endif
		mfree(storage);
	}
	mdynarray_dstr(&s_Threads);
	mshared_mutex_dstr(&s_ThreadsMtx);
	mthread_storage_free(s_ThreadStorageID);
}

mthread_t* mthread_current()
{
	mthread_storage_t* storage = (mthread_storage_t*) mthread_storage_get(s_ThreadStorageID);
	return storage ? storage->thread : NULL;
}

mthreadid_t mthread_current_id()
{
	mthread_storage_t* storage = (mthread_storage_t*) mthread_storage_get(s_ThreadStorageID);
	return storage ? storage->id : 0;
}

static bool mthread_wait_compare(volatile void* address, const void* expected, size_t expectedSize)
{
	switch (expectedSize)
	{
	case 1: return matomic8_load((volatile uint8_t*) address) == *(uint8_t*) expected;
	case 2: return matomic16_load((volatile uint16_t*) address) == *(uint8_t*) expected;
	case 4: return matomic32_load((volatile uint32_t*) address) == *(uint8_t*) expected;
	case 8: return matomic64_load((volatile uint64_t*) address) == *(uint8_t*) expected;
	default: return memcmp((const void*) address, expected, expectedSize) == 0;
	}
}

void mthread_wait_on_address(volatile void* address, const void* expected, size_t expectedSize)
{
	if (!address || !expected || !expectedSize)
		return;

	mthread_storage_t* storage = (mthread_storage_t*) mthread_storage_get(s_ThreadStorageID);
	if (!storage)
		return;

	while (mthread_wait_compare(address, expected, expectedSize))
	{
		storage->waitAddress      = address;
		storage->waitExpected     = expected;
		storage->waitExpectedSize = expectedSize;

#if BUILD_IS_SYSTEM_WINDOWS
		WaitForSingleObject(storage->waitEvent, ~0U);
#endif
	}
	storage->waitAddress      = NULL;
	storage->waitExpected     = NULL;
	storage->waitExpectedSize = 0;
}

void mthread_wake_by_address_one(volatile void* address)
{
	if (!address)
		return;

	mshared_mutex_shared_lock_busy(&s_ThreadsMtx);
	for (size_t i = 0; i < s_Threads.size; ++i)
	{
		mthread_storage_t* storage = *(mthread_storage_t**) mdynarray_get(&s_Threads, i);
		if (!storage)
			continue;

		if (storage->waitAddress != address ||
			mthread_wait_compare(storage->waitAddress, storage->waitExpected, storage->waitExpectedSize))
			continue;

#if BUILD_IS_SYSTEM_WINDOWS
		SetEvent(storage->waitEvent);
#else
#endif
		break;
	}
	mshared_mutex_shared_unlock_busy(&s_ThreadsMtx);
}

void mthread_wake_by_address_all(volatile void* address)
{
	if (!address)
		return;

	mshared_mutex_shared_lock_busy(&s_ThreadsMtx);
	for (size_t i = 0; i < s_Threads.size; ++i)
	{
		mthread_storage_t* storage = *(mthread_storage_t**) mdynarray_get(&s_Threads, i);
		if (!storage)
			continue;

		if (storage->waitAddress != address ||
			mthread_wait_compare(storage->waitAddress, storage->waitExpected, storage->waitExpectedSize))
			continue;

#if BUILD_IS_SYSTEM_WINDOWS
		SetEvent(storage->waitEvent);
#else
#endif
	}
	mshared_mutex_shared_unlock_busy(&s_ThreadsMtx);
}

size_t mthread_storage_alloc()
{
#if BUILD_IS_SYSTEM_WINDOWS
	return TlsAlloc();
#else
#endif
}

void mthread_storage_free(size_t index)
{
#if BUILD_IS_SYSTEM_WINDOWS
	TlsFree((DWORD) index);
#else
#endif
}

void* mthread_storage_get(size_t index)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return TlsGetValue((DWORD) index);
#else
#endif
}

void mthread_storage_set(size_t index, void* value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	TlsSetValue((DWORD) index, value);
#else
#endif
}

void mthread_sleep(size_t timeout)
{
#if BUILD_IS_SYSTEM_WINDOWS
	Sleep((DWORD) timeout);
#else
#endif
}

void mthread_exit(uint64_t exitCode)
{
	mthread_storage_t* storage = (mthread_storage_t*) mthread_storage_get(s_ThreadStorageID);
	if (!storage || !storage->thread)
		return;
	storage->thread->exitCode = exitCode;
	longjmp(storage->returnJmpBuf, 1);
}

mthread_t* mthread_new(mthreadfunc_t func, void* data)
{
	mthread_t* thread = (mthread_t*) mmalloc(sizeof(mthread_t));
	mthread_cstr(thread, func, data);
	return thread;
}

void mthread_del(mthread_t* self)
{
	if (!self)
		return;
	mthread_dstr(self);
	mfree(self);
}

bool mthread_cstr(mthread_t* self, mthreadfunc_t func, void* data)
{
	if (!self || !func)
		return false;

	self->func     = func;
	self->data     = data;
	self->native   = NULL;
	self->exitCode = 0;
	self->id       = 0;
	if (!mstring_cstr(&self->name, 0))
		return false;
#if BUILD_IS_SYSTEM_WINDOWS
	DWORD tid    = 0;
	self->native = (void*) CreateThread(NULL, 0, &mthread_thunk, self, CREATE_SUSPENDED, &tid);
	self->id     = tid;
#else
#endif
	return self->native != NULL;
}

void mthread_dstr(mthread_t* self)
{
	if (!self)
		return;

	if (self->native)
		mthread_join(self);
	mstring_dstr(&self->name);
}

void mthread_set_name(mthread_t* self, mstringview_t name)
{
	if (!self || !self->native)
		return;
	mstring_assign(&self->name, name);

#if BUILD_IS_SYSTEM_WINDOWS
	wchar_t wbuffer[32768];
	int     newLen  = MultiByteToWideChar(CP_UTF8, 0, mstring_begin(&self->name), (int) self->name.length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';
	SetThreadDescription(self->native, wbuffer);
#else
#endif
}

mstringview_t mthread_get_name(mthread_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mstring_substr(&self->name, 0, ~0ULL);
}

mthreadid_t mthread_get_id(mthread_t* self)
{
	return self ? self->id : 0;
}

bool mthread_join(mthread_t* self)
{
	if (!self || !self->native)
		return false;

#if BUILD_IS_SYSTEM_WINDOWS
	WaitForSingleObject(self->native, ~0U);
	CloseHandle(self->native);
	self->native = NULL;
	self->id     = 0;
#else
#endif
	return true;
}

void mthread_start(mthread_t* self)
{
	if (!self || !self->native)
		return;

#if BUILD_IS_SYSTEM_WINDOWS
	ResumeThread(self->native);
#else
#endif
}

void mwait_on_address(volatile void* address, const void* expected, size_t expectedSize)
{
	mthread_wait_on_address(address, expected, expectedSize);
}

void mwake_by_address_one(volatile void* address)
{
	mthread_wake_by_address_one(address);
}

void mwake_by_address_all(volatile void* address)
{
	mthread_wake_by_address_all(address);
}