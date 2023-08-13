#pragma once

typedef struct WLRTDynArray
{
	void*  data;
	size_t size;
	size_t capacity;
	size_t elementSize;
} WLRTDynArray;

bool WLRTDynArraySetup(WLRTDynArray* dynArray, size_t initialCapacity, size_t elementSize);
void WLRTDynArrayCleanup(WLRTDynArray* dynArray);
bool WLRTDynArrayResize(WLRTDynArray* dynArray, size_t newSize);
bool WLRTDynArrayReserve(WLRTDynArray* dynArray, size_t newCapacity);
bool WLRTDynArrayPushBack(WLRTDynArray* dynArray, const void* element);
bool WLRTDynArrayPushFront(WLRTDynArray* dynArray, const void* element);
bool WLRTDynArrayPopBack(WLRTDynArray* dynArray);
bool WLRTDynArrayPopFront(WLRTDynArray* dynArray);
bool WLRTDynArrayInsert(WLRTDynArray* dynArray, size_t index, const void* element);
bool WLRTDynArrayErase(WLRTDynArray* dynArray, size_t index);