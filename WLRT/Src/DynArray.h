#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct WLRTDynArray
{
	void*  data;
	size_t size;
	size_t capacity;
	size_t rowSize;
} WLRTDynArray;

typedef struct WLRTMultiDynArray
{
	void*   data;
	size_t  size;
	size_t  capacity;
	size_t  columns;
	size_t* rowSizes;
} WLRTMultiDynArray;

bool  WLRTDynArraySetup(WLRTDynArray* dynArray, size_t initialCapacity, size_t rowSize);
void  WLRTDynArrayCleanup(WLRTDynArray* dynArray);
void* WLRTDynArrayGet(WLRTDynArray* dynArray, size_t index);
bool  WLRTDynArrayClear(WLRTDynArray* dynArray);
bool  WLRTDynArrayResize(WLRTDynArray* dynArray, size_t newSize);
bool  WLRTDynArrayReserve(WLRTDynArray* dynArray, size_t newCapacity);
bool  WLRTDynArrayPushBack(WLRTDynArray* dynArray, const void* data);
bool  WLRTDynArrayPushFront(WLRTDynArray* dynArray, const void* data);
bool  WLRTDynArrayPopBack(WLRTDynArray* dynArray);
bool  WLRTDynArrayPopFront(WLRTDynArray* dynArray);
bool  WLRTDynArrayInsert(WLRTDynArray* dynArray, size_t index, const void* data);
bool  WLRTDynArrayErase(WLRTDynArray* dynArray, size_t index);

bool  WLRTMultiDynArraySetup(WLRTMultiDynArray* dynArray, size_t initialCapacity, const size_t* initialRowSizes, size_t initialColumns);
void  WLRTMultiDynArrayCleanup(WLRTMultiDynArray* dynArray);
void* WLRTMultiDynArrayGet(WLRTMultiDynArray* dynArray, size_t index, size_t column);
bool  WLRTMultiDynArrayClear(WLRTMultiDynArray* dynArray);
bool  WLRTMultiDynArrayResize(WLRTMultiDynArray* dynArray, size_t newSize);
bool  WLRTMultiDynArrayReserve(WLRTMultiDynArray* dynArray, size_t newCapacity);
bool  WLRTMultiDynArrayPushBack(WLRTMultiDynArray* dynArray, const void** data);
bool  WLRTMultiDynArrayPushFront(WLRTMultiDynArray* dynArray, const void** data);
bool  WLRTMultiDynArrayPopBack(WLRTMultiDynArray* dynArray);
bool  WLRTMultiDynArrayPopFront(WLRTMultiDynArray* dynArray);
bool  WLRTMultiDynArrayInsert(WLRTMultiDynArray* dynArray, size_t index, const void** data);
bool  WLRTMultiDynArrayErase(WLRTMultiDynArray* dynArray, size_t index);
// TODO(MarcasRealAccount): WLRTMultiDynArray, add functions for expanding/shrinking rows