#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define alignas(x) _Alignas(x)
#define alignof(x) _Alignof(x)

typedef struct mmalloc_stats_t
{
	uint64_t bytes;
	uint64_t count;
} mmalloc_stats_t;

typedef void* (*mmalloc_alloc_func)(size_t size, size_t alignment);
typedef void* (*mmalloc_realloc_func)(void* ptr, size_t newSize, size_t alignment);
typedef void (*mmalloc_free_func)(void* ptr, size_t alignment);
typedef size_t (*mmalloc_size_func)(void* ptr, size_t alignment);

typedef struct mmalloc_funcs_t
{
	mmalloc_alloc_func   alloc;
	mmalloc_realloc_func realloc;
	mmalloc_free_func    free;
	mmalloc_size_func    size;
} mmalloc_funcs_t;

void            mmalloc_set_funcs(mmalloc_funcs_t* funcs);
mmalloc_stats_t mmalloc_stats();

void*  mmalloc(size_t size);
void*  mrealloc(void* ptr, size_t newSize);
void   mfree(void* ptr);
size_t msize(void* ptr);

void*  mmalloc_aligned(size_t size, size_t alignment);
void*  mrealloc_aligned(void* ptr, size_t newSize, size_t alignment); // alignment is only a hint
void   mfree_aligned(void* ptr, size_t alignment);                    // alignment is only a hint
size_t msize_aligned(void* ptr, size_t alignment);                    // alignment is only a hint

uint64_t mbitscan_reverse(uint64_t value);
uint64_t mbit_ceil(uint64_t value);
uint64_t mbit_ceilfull(uint64_t value);
uint64_t malign_ceil(uint64_t value, uint64_t alignment);
uint64_t malign_floor(uint64_t value, uint64_t alignment);