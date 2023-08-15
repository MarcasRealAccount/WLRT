#include "DynArray.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool WLRTDynArraySetup(WLRTDynArray* dynArray, size_t initialCapacity, size_t elementSize)
{
	if (!dynArray || !elementSize)
		return 0;

	dynArray->data        = NULL;
	dynArray->size        = 0;
	dynArray->capacity    = initialCapacity;
	dynArray->elementSize = elementSize;
	if (dynArray->capacity)
	{
		dynArray->data = malloc(dynArray->capacity * dynArray->elementSize);
		if (!dynArray->data)
		{
			dynArray->capacity = 0;
			return false;
		}
	}
	return true;
}

void WLRTDynArrayCleanup(WLRTDynArray* dynArray)
{
	if (!dynArray)
		return;

	free(dynArray->data);
	dynArray->data     = NULL;
	dynArray->size     = 0;
	dynArray->capacity = 0;
}

void* WLRTDynArrayGet(WLRTDynArray* dynArray, size_t element)
{
	return dynArray && dynArray->data && element < dynArray->capacity ? (void*) (((uint8_t*) dynArray->data) + element * dynArray->elementSize) : NULL;
}

bool WLRTDynArrayClear(WLRTDynArray* dynArray)
{
	if (!dynArray || !dynArray->data)
		return false;
	memset(dynArray->data, 0, dynArray->size * dynArray->elementSize);
	dynArray->size = 0;
	return true;
}

static bool WLRTDynArrayEnsureCapacity(WLRTDynArray* dynArray, size_t capacity)
{
	if (capacity < dynArray->capacity)
		return true;

	--capacity;
	capacity |= capacity >> 1;
	capacity |= capacity >> 2;
	capacity |= capacity >> 4;
	capacity |= capacity >> 8;
	capacity |= capacity >> 16;
	capacity |= capacity >> 32;
	++capacity;
	if (capacity == dynArray->capacity)
		return true;

	void* newData = malloc(capacity * dynArray->elementSize);
	if (!newData)
		return false;
	size_t toCopy = min(capacity, dynArray->capacity) * dynArray->elementSize;
	memcpy(newData, dynArray->data, toCopy);
	memset((uint8_t*) newData + toCopy, 0, capacity - toCopy);
	free(dynArray->data);
	dynArray->data     = newData;
	dynArray->capacity = capacity;
	return true;
}

bool WLRTDynArrayResize(WLRTDynArray* dynArray, size_t newSize)
{
	if (!dynArray)
		return false;

	if (newSize < dynArray->size)
	{
		memset((uint8_t*) dynArray->data + newSize * dynArray->elementSize, 0, (dynArray->size - newSize) * dynArray->elementSize);
		dynArray->size = newSize;
		return true;
	}

	if (newSize > dynArray->capacity &&
		!WLRTDynArrayReserve(dynArray, newSize))
		return false;

	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->elementSize, 0, (newSize - dynArray->size) * dynArray->elementSize);
	dynArray->size = newSize;
	return true;
}

bool WLRTDynArrayReserve(WLRTDynArray* dynArray, size_t newCapacity)
{
	if (!dynArray || newCapacity < dynArray->size)
		return false;

	return WLRTDynArrayEnsureCapacity(dynArray, newCapacity);
}

bool WLRTDynArrayPushBack(WLRTDynArray* dynArray, const void* element)
{
	if (!dynArray ||
		!WLRTDynArrayEnsureCapacity(dynArray, dynArray->size + 1))
		return false;

	void* data = WLRTDynArrayGet(dynArray, dynArray->size);
	memcpy(data, element, dynArray->elementSize);
	++dynArray->size;
	return true;
}

bool WLRTDynArrayPushFront(WLRTDynArray* dynArray, const void* element)
{
	if (!dynArray ||
		!WLRTDynArrayEnsureCapacity(dynArray, dynArray->size + 1))
		return false;

	memmove((uint8_t*) dynArray->data + dynArray->elementSize, dynArray->data, dynArray->size * dynArray->elementSize);
	memcpy(dynArray->data, element, dynArray->elementSize);
	++dynArray->size;
	return true;
}

bool WLRTDynArrayPopBack(WLRTDynArray* dynArray)
{
	if (!dynArray)
		return false;

	if (dynArray->size == 0)
		return true;
	memmove(dynArray->data, (uint8_t*) dynArray->data + dynArray->elementSize, (dynArray->size - 1) * dynArray->elementSize);
	--dynArray->size;
	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->elementSize, 0, dynArray->elementSize);
	return true;
}

bool WLRTDynArrayPopFront(WLRTDynArray* dynArray)
{
	if (!dynArray)
		return false;

	if (dynArray->size == 0)
		return true;
	--dynArray->size;
	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->elementSize, 0, dynArray->elementSize);
	return true;
}

bool WLRTDynArrayInsert(WLRTDynArray* dynArray, size_t index, const void* element)
{
	if (!dynArray ||
		!WLRTDynArrayEnsureCapacity(dynArray, index + 1))
		return false;

	memmove((uint8_t*) dynArray->data + (index + 1) * dynArray->elementSize, (uint8_t*) dynArray->data + index * dynArray->elementSize, (dynArray->size - index) * dynArray->elementSize);
	memcpy((uint8_t*) dynArray->data + index * dynArray->elementSize, element, dynArray->elementSize);
	++dynArray->size;
	return true;
}

bool WLRTDynArrayErase(WLRTDynArray* dynArray, size_t index)
{
	if (!dynArray)
		return false;

	memmove((uint8_t*) dynArray->data + index * dynArray->elementSize, (uint8_t*) dynArray->data + (index + 1) * dynArray->elementSize, (dynArray->size - index - 1) * dynArray->elementSize);
	--dynArray->size;
	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->elementSize, 0, dynArray->elementSize);
	return true;
}