#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef int (*WLRTThreadCallbackFn)(void* userData);
typedef struct WLRTThreadData
{
	WLRTThreadCallbackFn callback;
	void*                userData;

	size_t nameLen;
	char*  nameBuf;

	void* native;
	int   exitCode;
} WLRTThreadData;

bool        WLRTThreadSetup(WLRTThreadData* thread);
void        WLRTThreadCleanup(WLRTThreadData* thread);
bool        WLRTThreadJoin(WLRTThreadData* thread, int* exitCode);
void        WLRTThreadStart(WLRTThreadData* thread);
void        WLRTThreadSetName(WLRTThreadData* thread, const char* name, size_t length);
const char* WLRTThreadGetName(WLRTThreadData* thread);