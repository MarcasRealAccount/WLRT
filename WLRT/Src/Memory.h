#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct WLRTAllocationStats
{
	uint64_t totalBytes;
	uint64_t totalAllocs;
} WLRTAllocationStats;

WLRTAllocationStats WLRTAllocStats();

size_t WLRTAlignUp(size_t x, size_t alignment);

size_t WLRTAllocSize(void* ptr, size_t alignment);
void*  WLRTAlloc(size_t size, size_t alignment);
void   WLRTFree(void* ptr, size_t alignment);

#define alignof(x) _Alignof(x)