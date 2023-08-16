#pragma once

#include <stddef.h>

size_t WLRTAlignUp(size_t x, size_t alignment);

void* WLRTAlloc(size_t size, size_t alignment);
void  WLRTFree(void* ptr, size_t alignment);

#define alignof(x) _Alignof(x)