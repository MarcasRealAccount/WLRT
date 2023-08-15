#include "Threading.h"

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

static DWORD WLRTThreadThunk(LPVOID data)
{
	WLRTThreadData* thread = (WLRTThreadData*) data;
	return (DWORD) thread->callback(thread->userData);
}

bool WLRTThreadSetup(WLRTThreadData* thread)
{
	if (!thread || !thread->callback)
		return false;

	thread->nameLen  = 0;
	thread->nameBuf  = NULL;
	thread->native   = (void*) CreateThread(NULL, 0, &WLRTThreadThunk, thread, CREATE_SUSPENDED, NULL);
	thread->exitCode = 0;
	return thread->native != NULL;
}

void WLRTThreadCleanup(WLRTThreadData* thread)
{
	if (!thread)
		return;

	if (thread->native)
	{
		WaitForSingleObject((HANDLE) thread->native, ~0U);
		DWORD exitCode = 0;
		GetExitCodeThread((HANDLE) thread->native, &exitCode);
		thread->exitCode = (int) exitCode;
		CloseHandle((HANDLE) thread->native);
		thread->native = NULL;
	}
}

bool WLRTThreadJoin(WLRTThreadData* thread, int* exitCode)
{
	if (exitCode) *exitCode = 0;
	if (!thread || !thread->native)
		return false;

	WaitForSingleObject((HANDLE) thread->native, ~0U);
	DWORD dwExitCode = 0;
	if (!GetExitCodeThread((HANDLE) thread->native, &dwExitCode))
		return false;
	thread->exitCode = (int) dwExitCode;
	if (exitCode) *exitCode = (int) dwExitCode;
	return true;
}

void WLRTThreadStart(WLRTThreadData* thread)
{
	if (!thread || !thread->native)
		return;

	ResumeThread((HANDLE) thread->native);
}

void WLRTThreadSetName(WLRTThreadData* thread, const char* name, size_t length)
{
	if (!thread)
		return;

	if (thread->nameBuf)
		free(thread->nameBuf);

	if (length == ~0ULL)
		length = strlen(name);
	thread->nameLen = length;
	thread->nameBuf = malloc((length + 1) * sizeof(char));
	if (!thread->nameBuf)
	{
		thread->nameLen = 0;
		return;
	}
	memcpy(thread->nameBuf, name, thread->nameLen);
	thread->nameBuf[thread->nameLen] = '\0';

	if (thread->native)
	{
		wchar_t* buf = malloc((thread->nameLen + 1) * sizeof(wchar_t));
		if (!buf)
			return;

		size_t written = mbstowcs(buf, thread->nameBuf, thread->nameLen);
		buf[written]   = L'\0';
		SetThreadDescription((HANDLE) thread->native, buf);
		free(buf);
	}
}

const char* WLRTThreadGetName(WLRTThreadData* thread)
{
	if (!thread)
		return NULL;
	return thread->nameBuf ? thread->nameBuf : "Thread";
}

bool WLRTMutexSetup(WLRTMutex* mutex)
{
	mutex->inUse = 0;
	return true;
}

void WLRTMutexCleanup(WLRTMutex* mutex)
{
	(void) mutex;
}

void WLRTMutexLock(WLRTMutex* mutex)
{
	uint32_t val;
	while ((val = InterlockedCompareExchange((volatile LONG*) &mutex->inUse, 1, 0)) == 1)
		WaitOnAddress(&mutex->inUse, &val, 4, INFINITE);
}

bool WLRTMutexTryLock(WLRTMutex* mutex)
{
	return InterlockedCompareExchange((volatile LONG*) &mutex->inUse, 1, 0) == 0;
}

void WLRTMutexUnlock(WLRTMutex* mutex)
{
	if (InterlockedExchange((volatile LONG*) &mutex->inUse, 0) == 1)
		WakeByAddressSingle(&mutex->inUse);
}