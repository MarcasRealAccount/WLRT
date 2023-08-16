#include "Memory.h"
#include "Build.h"

#include <stdlib.h>

size_t WLRTAlignUp(size_t x, size_t alignment)
{
	size_t mask = alignment - 1;
	return (mask & alignment) == 0 ? (x + mask) & ~mask : (x + mask) / alignment * alignment;
}

void* WLRTAlloc(size_t size, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return _aligned_malloc(size, alignment);
#else
	return aligned_alloc(alignment, size);
#endif
}

void WLRTFree(void* ptr, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	(void) alignment;
	_aligned_free(ptr);
#else
	(void) alignment;
	free(ptr);
#endif
}