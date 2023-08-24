#include "mem.h"
#include "atomic.h"
#include "build.h"

#include <stdint.h>

#if BUILD_IS_TOOLSET_MSVC
	#include <intrin.h>
#elif BUILD_IS_SYSTEM_MACOSX
	#include <malloc/malloc.h>
	#include <stdlib.h>
#else
	#include <stdlib.h>
#endif

static void* mdefault_alloc(size_t size, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return _aligned_malloc(size, alignment);
#else
	return aligned_alloc(alignment, size);
#endif
}

static void* mdefault_realloc(void* ptr, size_t newSize, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return _aligned_realloc(ptr, newSize, alignment);
#else
	(void) alignment;
	return realloc(ptr, newSize);
#endif
}

static void mdefault_free(void* ptr, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	(void) alignment;
	_aligned_free(ptr);
#else
	(void) alignment;
	free(ptr);
#endif
}

static size_t mdefault_size(void* ptr, size_t alignment)
{
#if BUILD_IS_SYSTEM_WINDOWS
	(void) alignment;
	return _aligned_msize(ptr, alignment, 0);
#elif BUILD_IS_SYSTEM_MACOSX
	(void) alignment;
	return malloc_size(ptr);
#else
	(void) alignment;
	return malloc_usable_size(ptr);
#endif
}

static mmalloc_stats_t s_Stats = {
	.bytes = 0,
	.count = 0
};

static struct
{
	void* alloc;
	void* realloc;
	void* free;
	void* size;
} s_Funcs = {
	.alloc   = (void*) &mdefault_alloc,
	.realloc = (void*) &mdefault_realloc,
	.free    = (void*) &mdefault_free,
	.size    = (void*) &mdefault_size
};

void mmalloc_set_funcs(mmalloc_funcs_t* funcs)
{
	matomicptr_store(&s_Funcs.alloc, (void*) funcs->alloc);
	matomicptr_store(&s_Funcs.realloc, (void*) funcs->realloc);
	matomicptr_store(&s_Funcs.free, (void*) funcs->free);
	matomicptr_store(&s_Funcs.size, (void*) funcs->size);
}

mmalloc_stats_t mmalloc_stats()
{
	mmalloc_stats_t stats = {
		.bytes = matomic64_load(&s_Stats.bytes),
		.count = matomic64_load(&s_Stats.count)
	};
	return stats;
}

void* mmalloc(size_t size)
{
	return mmalloc_aligned(size, 16);
}

void* mrealloc(void* ptr, size_t newSize)
{
	return mrealloc_aligned(ptr, newSize, 16);
}

void mfree(void* ptr)
{
	mfree_aligned(ptr, 16);
}

size_t msize(void* ptr)
{
	return msize_aligned(ptr, 16);
}

void* mmalloc_aligned(size_t size, size_t alignment)
{
	void*  ptr = ((mmalloc_alloc_func) matomicptr_load(&s_Funcs.alloc))(size, alignment);
	size_t sz  = msize_aligned(ptr, alignment);
	matomic64_fetch_add(&s_Stats.bytes, sz);
	matomic64_fetch_add(&s_Stats.count, 1);
	return ptr;
}

void* mrealloc_aligned(void* ptr, size_t newSize, size_t alignment)
{
	uint64_t oldSize = msize_aligned(ptr, alignment);
	void*    newPtr  = ((mmalloc_realloc_func) matomicptr_load(&s_Funcs.realloc))(ptr, newSize, alignment);
	uint64_t size    = msize_aligned(newPtr, alignment);
	matomic64_fetch_add(&s_Stats.bytes, size - oldSize);
	return newPtr;
}

void mfree_aligned(void* ptr, size_t alignment)
{
	size_t size = msize_aligned(ptr, alignment);
	((mmalloc_free_func) matomicptr_load(&s_Funcs.free))(ptr, alignment);
	matomic64_fetch_sub(&s_Stats.bytes, size);
	matomic64_fetch_sub(&s_Stats.count, 1);
}

size_t msize_aligned(void* ptr, size_t alignment)
{
	return ((mmalloc_size_func) matomicptr_load(&s_Funcs.size))(ptr, alignment);
}

uint64_t mbitscan_reverse(uint64_t value)
{
#if BUILD_IS_TOOLSET_MSVC
	unsigned long index;
	_BitScanReverse64(&index, (unsigned __int64) value);
	return (uint64_t) index;
#elif BUILD_IS_TOOLSET_GCC || BUILD_IS_TOOLSET_CLANG
	return __builtin_clzll(value) ^ 63;
#else
	uint64_t mask = 0x8000000000000000ULL;
	for (uint8_t i = 63; i != 0xFF; --i, mask >>= 1)
		if (value & mask)
			return i;
	return 0;
#endif
}

uint64_t mbit_ceil(uint64_t value)
{
	uint64_t temp = 2ULL << mbitscan_reverse(value - 1);
	return value <= 1 ? 1 : temp;
}

uint64_t mbit_ceilfull(uint64_t value)
{
	uint64_t temp = mbit_ceil(value);
	return value > 0x8000000000000000ULL ? ~0ULL : temp;
}

uint64_t malign_ceil(uint64_t value, uint64_t alignment)
{
	uint64_t mask = alignment - 1;
	return alignment & mask ? (value + mask) / alignment * alignment : (value + mask) & ~mask;
}

uint64_t malign_floor(uint64_t value, uint64_t alignment)
{
	uint64_t mask = alignment - 1;
	return alignment & mask ? value / alignment * alignment : value & ~mask;
}

// MSVC
//	mbitceil:
//		lea		rax,	QWORD PTR [rcx-1]
//		mov		rdx,	rcx
//		bsr		rcx,	rax
//		mov		eax,	2
//		shl		rax,	cl
//		mov		ecx,	1
//		cmp		rdx,	rcx
//		cmovbe	rax,	rcx
//		ret		0
//
//	mbitceilfull:
//		mov		rdx,	rcx
//		lea		rax,	QWORD PTR [rcx-1]
//		bsr		rcx,	rax
//		mov		eax,	2
//		mov		r8,		-1
//		shl		rax,	cl
//		mov		ecx,	1
//		cmp		rdx,	rcx
//		cmovbe	rax,	rcx
//		mov		rcx,	0x8000'0000'0000'0000
//		cmp		rdx,	rcx
//		cmova	rax,	r8
//		ret		0
//
// Clang
//	mbitceil:
//		lea		rax,	[rdi - 1]
//		bsr		rcx,	rax
//		mov		edx,	2
//		shl		rdx,	cl
//		cmp		rdi,	2
//		mov		eax,	1
//		cmovae	rax,	rdx
//		ret
//
//	mbitceilfull:
//		lea		rax,	[rdi - 1]
//		bsr		rcx,	rax
//		mov		eax,	2
//		shl		rax,	cl
//		cmp		rdi,	2
//		mov		ecx,	1
//		cmovae	rcx,	rax
//		movabs	rax,	0x8000'0000'0000'0000
//		cmp		rdi,	rax
//		mov		rax,	-1
//		cmovbe	rax,	rcx
//		ret
//
// GCC
//	mbitceil:
//		mov		eax,	1
//		cmp		rdi,	1
//		jbe		.L3
//		sub		rdi,	1
//		mov		eax,	2
//		bsr		rcx,	rdi
//		sal		rax,	cl
//	.L3:
//		ret
//
//	mbitceilfull:
//		mov		eax,	1
//		cmp		rdi,	1
//		jbe		.L6
//		movabs	rdx,	0x8000'0000'0000'0000
//		mov		rax,	-1
//		cmp		rdx,	rdi
//		jb		.L6
//		sub		rdi,	1
//		mov		eax,	2
//		bsr		rcx,	rdi
//		sal		rax,	cl
//	.L6:
//		ret
