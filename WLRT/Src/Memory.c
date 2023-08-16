#include "Memory.h"
#include "Build.h"
#include "Threading.h"

#include <stdint.h>
#include <stdlib.h>

WLRTAllocationStats s_AllocStats = {
	.totalBytes  = 0,
	.totalAllocs = 0
};

WLRTAllocationStats WLRTAllocStats()
{
	WLRTAllocationStats result = {
		.totalBytes  = WLRTAtomic64Read(&s_AllocStats.totalBytes),
		.totalAllocs = WLRTAtomic64Read(&s_AllocStats.totalAllocs)
	};
	return result;
}

size_t WLRTAlignUp(size_t x, size_t alignment)
{
	size_t mask = alignment - 1;
	return (mask & alignment) == 0 ? (x + mask) & ~mask : (x + mask) / alignment * alignment;
}

size_t WLRTAllocSize(void* ptr, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	(void) alignment;
	return _msize(ptr);
#else
	(void) alignment;
	return malloc_usable_size(ptr);
#endif
}

void* WLRTAlloc(size_t size, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	void* mem = _aligned_malloc(size, alignment);
	if (!mem)
		return NULL;
#else
	void* mem = aligned_alloc(alignment, size);
	if (!mem)
		return NULL;
#endif
	WLRTAtomic64Add(&s_AllocStats.totalAllocs, 1);
	WLRTAtomic64Add(&s_AllocStats.totalBytes, WLRTAllocSize(mem, alignment));
	return mem;
}

void WLRTFree(void* ptr, size_t alignment)
{
	if (!ptr)
		return;

	size_t allocSize = WLRTAllocSize(ptr, alignment);
#if BUILD_IS_SYSTEM_WINDOWS
	_aligned_free(ptr);
#else
	free(ptr);
#endif
	if (allocSize == 0)
		return;
	WLRTAtomic64Sub(&s_AllocStats.totalBytes, allocSize);
	WLRTAtomic64Sub(&s_AllocStats.totalAllocs, 1);
}