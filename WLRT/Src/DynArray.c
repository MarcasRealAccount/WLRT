#include "DynArray.h"
#include "Memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool WLRTDynArraySetup(WLRTDynArray* dynArray, size_t initialCapacity, size_t rowSize)
{
	if (!dynArray || !rowSize)
		return 0;

	dynArray->data     = NULL;
	dynArray->size     = 0;
	dynArray->capacity = initialCapacity;
	dynArray->rowSize  = rowSize;
	if (dynArray->capacity)
	{
		dynArray->data = WLRTAlloc(dynArray->capacity * dynArray->rowSize, 16);
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

	WLRTFree(dynArray->data, 16);
	dynArray->data     = NULL;
	dynArray->size     = 0;
	dynArray->capacity = 0;
}

void* WLRTDynArrayGet(WLRTDynArray* dynArray, size_t element)
{
	return dynArray && dynArray->data && element < dynArray->capacity ? (void*) (((uint8_t*) dynArray->data) + element * dynArray->rowSize) : NULL;
}

bool WLRTDynArrayClear(WLRTDynArray* dynArray)
{
	if (!dynArray || !dynArray->data)
		return false;
	memset(dynArray->data, 0, dynArray->size * dynArray->rowSize);
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

	void* newData = WLRTAlloc(capacity * dynArray->rowSize, 16);
	if (!newData)
		return false;
	size_t toCopy = min(capacity, dynArray->capacity);
	memcpy(newData, dynArray->data, toCopy * dynArray->rowSize);
	memset((uint8_t*) newData + toCopy * dynArray->rowSize, 0, (capacity - toCopy) * dynArray->rowSize);
	WLRTFree(dynArray->data, 16);
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
		memset((uint8_t*) dynArray->data + newSize * dynArray->rowSize, 0, (dynArray->size - newSize) * dynArray->rowSize);
		dynArray->size = newSize;
		return true;
	}

	if (newSize > dynArray->capacity &&
		!WLRTDynArrayReserve(dynArray, newSize))
		return false;

	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->rowSize, 0, (newSize - dynArray->size) * dynArray->rowSize);
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
	memcpy(data, element, dynArray->rowSize);
	++dynArray->size;
	return true;
}

bool WLRTDynArrayPushFront(WLRTDynArray* dynArray, const void* element)
{
	if (!dynArray ||
		!WLRTDynArrayEnsureCapacity(dynArray, dynArray->size + 1))
		return false;

	memmove((uint8_t*) dynArray->data + dynArray->rowSize, dynArray->data, dynArray->size * dynArray->rowSize);
	memcpy(dynArray->data, element, dynArray->rowSize);
	++dynArray->size;
	return true;
}

bool WLRTDynArrayPopBack(WLRTDynArray* dynArray)
{
	if (!dynArray)
		return false;

	if (dynArray->size == 0)
		return true;
	--dynArray->size;
	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->rowSize, 0, dynArray->rowSize);
	return true;
}

bool WLRTDynArrayPopFront(WLRTDynArray* dynArray)
{
	if (!dynArray)
		return false;

	if (dynArray->size == 0)
		return true;
	--dynArray->size;
	memmove(dynArray->data, (uint8_t*) dynArray->data + dynArray->rowSize, dynArray->size * dynArray->rowSize);
	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->rowSize, 0, dynArray->rowSize);
	return true;
}

bool WLRTDynArrayInsert(WLRTDynArray* dynArray, size_t index, const void* element)
{
	if (!dynArray ||
		!WLRTDynArrayEnsureCapacity(dynArray, index + 1))
		return false;

	memmove((uint8_t*) dynArray->data + (index + 1) * dynArray->rowSize, (uint8_t*) dynArray->data + index * dynArray->rowSize, (dynArray->size - index) * dynArray->rowSize);
	memcpy((uint8_t*) dynArray->data + index * dynArray->rowSize, element, dynArray->rowSize);
	++dynArray->size;
	return true;
}

bool WLRTDynArrayErase(WLRTDynArray* dynArray, size_t index)
{
	if (!dynArray)
		return false;

	memmove((uint8_t*) dynArray->data + index * dynArray->rowSize, (uint8_t*) dynArray->data + (index + 1) * dynArray->rowSize, (dynArray->size - index - 1) * dynArray->rowSize);
	--dynArray->size;
	memset((uint8_t*) dynArray->data + dynArray->size * dynArray->rowSize, 0, dynArray->rowSize);
	return true;
}

bool WLRTMultiDynArraySetup(WLRTMultiDynArray* dynArray, size_t initialCapacity, const size_t* initialRowSizes, size_t initialColumns)
{
	if (!dynArray || !initialRowSizes || !initialColumns)
		return false;

	dynArray->size     = 0;
	dynArray->capacity = initialCapacity;
	dynArray->columns  = initialColumns;
	dynArray->rowSizes = (size_t*) WLRTAlloc(initialColumns * sizeof(size_t), alignof(size_t));
	if (!dynArray->rowSizes)
	{
		WLRTMultiDynArrayCleanup(dynArray);
		return false;
	}
	memcpy(dynArray->rowSizes, initialRowSizes, dynArray->columns * sizeof(size_t));
	size_t totalSize = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
		totalSize += WLRTAlignUp(dynArray->rowSizes[i] * initialCapacity, 16);
	dynArray->data = WLRTAlloc(totalSize, 16);
	if (!dynArray->data)
	{
		WLRTMultiDynArrayCleanup(dynArray);
		return false;
	}
	memset(dynArray->data, 0, totalSize * initialCapacity);
	return true;
}

void WLRTMultiDynArrayCleanup(WLRTMultiDynArray* dynArray)
{
	if (!dynArray)
		return;
	WLRTFree(dynArray->data, 16);
	WLRTFree(dynArray->rowSizes, alignof(size_t));
	dynArray->data     = NULL;
	dynArray->size     = 0;
	dynArray->capacity = 0;
	dynArray->columns  = 0;
	dynArray->rowSizes = NULL;
}

void* WLRTMultiDynArrayGet(WLRTMultiDynArray* dynArray, size_t index, size_t column)
{
	if (!dynArray || column >= dynArray->columns || index >= dynArray->capacity)
		return NULL;

	size_t offset = 0;
	for (size_t i = 0; i < column; ++i)
		offset += WLRTAlignUp(dynArray->rowSizes[i] * dynArray->capacity, 16);
	offset += dynArray->rowSizes[column] * index;
	return (uint8_t*) dynArray->data + offset;
}

bool WLRTMultiDynArrayClear(WLRTMultiDynArray* dynArray)
{
	if (!dynArray)
		return false;

	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memset((uint8_t*) dynArray->data + offset, 0, dynArray->size * dynArray->rowSizes[i]);
		offset += dynArray->rowSizes[i] * dynArray->capacity;
	}
	dynArray->size = 0;
	return true;
}

static bool WLRTMultiDynArrayEnsureCapacity(WLRTMultiDynArray* dynArray, size_t capacity)
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

	size_t totalSize = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
		totalSize += WLRTAlignUp(dynArray->rowSizes[i] * capacity, 16);

	void* newData = WLRTAlloc(totalSize, 16);
	if (!newData)
		return false;
	size_t toCopy  = min(capacity, dynArray->capacity);
	size_t offset1 = 0;
	size_t offset2 = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memcpy((uint8_t*) newData + offset1, (uint8_t*) dynArray->data + offset2, toCopy * dynArray->rowSizes[i]);
		memset((uint8_t*) newData + offset1 + toCopy * dynArray->rowSizes[i], 0, (capacity - toCopy) * dynArray->rowSizes[i]);
		offset1 += WLRTAlignUp(dynArray->rowSizes[i] * capacity, 16);
		offset2 += WLRTAlignUp(dynArray->rowSizes[i] * dynArray->capacity, 16);
	}
	WLRTFree(dynArray->data, 16);
	dynArray->data     = newData;
	dynArray->capacity = capacity;
	return true;
}

bool WLRTMultiDynArrayResize(WLRTMultiDynArray* dynArray, size_t newSize)
{
	if (!dynArray)
		return false;

	if (newSize < dynArray->size)
	{
		size_t offset = 0;
		for (size_t i = 0; i < dynArray->columns; ++i)
		{
			memset((uint8_t*) dynArray->data + offset + newSize * dynArray->rowSizes[i], 0, (dynArray->size - newSize) * dynArray->rowSizes[i]);
			offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
		}
		dynArray->size = newSize;
		return true;
	}

	if (newSize > dynArray->capacity &&
		!WLRTMultiDynArrayReserve(dynArray, newSize))
		return false;

	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memset((uint8_t*) dynArray->data + offset + newSize * dynArray->rowSizes[i], 0, (dynArray->size - newSize) * dynArray->rowSizes[i]);
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	}
	dynArray->size = newSize;
	return true;
}

bool WLRTMultiDynArrayReserve(WLRTMultiDynArray* dynArray, size_t newCapacity)
{
	if (!dynArray || newCapacity < dynArray->size)
		return false;

	return WLRTMultiDynArrayEnsureCapacity(dynArray, newCapacity);
}

bool WLRTMultiDynArrayPushBack(WLRTMultiDynArray* dynArray, const void** data)
{
	if (!dynArray ||
		!data ||
		!WLRTMultiDynArrayEnsureCapacity(dynArray, dynArray->size + 1))
		return false;

	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memcpy((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * dynArray->size, data[i], dynArray->rowSizes[i]);
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	}
	++dynArray->size;
	return true;
}

bool WLRTMultiDynArrayPushFront(WLRTMultiDynArray* dynArray, const void** data)
{
	if (!dynArray ||
		!data ||
		!WLRTMultiDynArrayEnsureCapacity(dynArray, dynArray->size + 1))
		return false;

	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns - 1; ++i)
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	for (size_t i = dynArray->columns; i > 0; --i)
	{
		memmove((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i - 1], (uint8_t*) dynArray->data + offset, dynArray->size * dynArray->rowSizes[i - 1]);
		if (i > 1)
			offset -= WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i - 2], 16);
	}

	offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memcpy((uint8_t*) dynArray->data + offset, data[i], dynArray->rowSizes[i]);
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	}
	++dynArray->size;
	return true;
}

bool WLRTMultiDynArrayPopBack(WLRTMultiDynArray* dynArray)
{
	if (!dynArray)
		return false;

	if (dynArray->size == 0)
		return true;

	--dynArray->size;
	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memset((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * dynArray->size, 0, dynArray->rowSizes[i]);
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	}
	return true;
}

bool WLRTMultiDynArrayPopFront(WLRTMultiDynArray* dynArray)
{
	if (!dynArray)
		return false;

	if (dynArray->size == 0)
		return true;

	--dynArray->size;
	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns - 1; ++i)
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	for (size_t i = dynArray->columns; i > 0; --i)
	{
		memmove((uint8_t*) dynArray->data + offset, (uint8_t*) dynArray->data + offset + dynArray->rowSizes[i - 1], dynArray->size * dynArray->rowSizes[i - 1]);
		if (i > 1)
			offset -= WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i - 2], 16);
	}
	offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memset((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * dynArray->size, 0, dynArray->rowSizes[i]);
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	}
	return true;
}

bool WLRTMultiDynArrayInsert(WLRTMultiDynArray* dynArray, size_t index, const void** data)
{
	if (!dynArray ||
		!data ||
		!WLRTMultiDynArrayEnsureCapacity(dynArray, dynArray->size + 1))
		return false;

	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns - 1; ++i)
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	for (size_t i = dynArray->columns; i > 0; --i)
	{
		memmove((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * (index + 1), (uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * index, (dynArray->size - index) * dynArray->rowSizes[i]);
		if (i > 1)
			offset -= WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i - 2], 16);
	}
	offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memcpy((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * index, data[i], dynArray->rowSizes[i]);
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i - 2], 16);
	}
	++dynArray->size;
	return true;
}

bool WLRTMultiDynArrayErase(WLRTMultiDynArray* dynArray, size_t index)
{
	if (!dynArray)
		return false;

	--dynArray->size;
	size_t offset = 0;
	for (size_t i = 0; i < dynArray->columns - 1; ++i)
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i], 16);
	for (size_t i = dynArray->columns; i > 0; --i)
	{
		memmove((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * (index - 1), (uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * index, (dynArray->size - index) * dynArray->rowSizes[i]);
		if (i > 1)
			offset -= WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i - 2], 16);
	}
	offset = 0;
	for (size_t i = 0; i < dynArray->columns; ++i)
	{
		memset((uint8_t*) dynArray->data + offset + dynArray->rowSizes[i] * dynArray->size, 0, dynArray->rowSizes[i]);
		offset += WLRTAlignUp(dynArray->capacity * dynArray->rowSizes[i - 2], 16);
	}
	return true;
}