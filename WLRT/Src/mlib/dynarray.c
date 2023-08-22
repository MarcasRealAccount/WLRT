#include "dynarray.h"
#include "mem.h"

#include <stdint.h>
#include <stdlib.h>

mdynarray_t mdynarray(size_t initialCapacity, size_t rowSize)
{
	mdynarray_t arr;
	memset(&arr, 0, sizeof(arr));
	mdynarray_cstr(&arr, initialCapacity, rowSize);
	return arr;
}

mdynarray_t* mdynarray_new(size_t initialCapacity, size_t rowSize)
{
	mdynarray_t* arr = (mdynarray_t*) mmalloc(sizeof(mdynarray_t));
	if (!arr)
		return NULL;
	mdynarray_cstr(arr, initialCapacity, rowSize);
	return arr;
}

void mdynarray_del(mdynarray_t* self)
{
	if (!self)
		return;
	mdynarray_dstr(self);
	mfree(self);
}

bool mdynarray_cstr(mdynarray_t* self, size_t initialCapacity, size_t rowSize)
{
	if (!self)
		return false;
	self->data     = NULL;
	self->size     = 0;
	self->capacity = 0;
	self->rowSize  = rowSize;
	return mdynarray_reserve(self, initialCapacity);
}

void mdynarray_dstr(mdynarray_t* self)
{
	if (!self)
		return;
	mfree(self->data);
	self->data     = NULL;
	self->size     = 0;
	self->capacity = 0;
	self->rowSize  = 0;
}

bool mdynarray_copy(mdynarray_t* self, const mdynarray_t* copy)
{
	if (!self || !copy)
		return false;
	mdynarray_dstr(self);
	if (!mdynarray_cstr(self, copy->size, copy->rowSize))
		return false;
	memcpy(self->data, copy->data, copy->size * copy->rowSize);
	self->size = copy->size;
	return true;
}

bool mdynarray_clear(mdynarray_t* self)
{
	if (!self || !self->data)
		return false;

	if (self->data)
		memset(self->data, 0, self->size * self->rowSize);
	return true;
}

bool mdynarray_resize(mdynarray_t* self, size_t newSize)
{
	if (!self ||
		!mdynarray_reserve(self, newSize))
		return false;
	if (self->size < newSize)
		memset((uint8_t*) self->data + newSize, 0, self->size - newSize);
	self->size = newSize;
	return true;
}

bool mdynarray_reserve(mdynarray_t* self, size_t newCapacity)
{
	if (!self)
		return false;
	newCapacity = mbit_ceilfull(newCapacity);
	if (newCapacity <= self->capacity)
		return true;

	void* newData = mmalloc(newCapacity * self->rowSize);
	if (!newData)
		return false;
	if (self->data)
	{
		memcpy(newData, self->data, self->size * self->rowSize);
		memset((uint8_t*) newData + (self->size + 1) * self->rowSize, 0, (newCapacity - self->size) * self->rowSize);
		mfree(self->data);
	}
	else
	{
		memset(newData, 0, newCapacity * self->rowSize);
	}
	self->data     = newData;
	self->capacity = newCapacity;
	return true;
}

bool mdynarray_shrink_to_fit(mdynarray_t* self)
{
	if (!self)
		return false;
	size_t newCapacity = mbit_ceilfull(self->size);
	if (newCapacity == self->capacity)
		return true;

	void* newData = mmalloc(newCapacity * self->rowSize);
	if (!newData)
		return false;
	if (self->data)
	{
		memcpy(newData, self->data, self->size * self->rowSize);
		memset((uint8_t*) newData + (self->size + 1) * self->rowSize, 0, (newCapacity - self->size) * self->rowSize);
		mfree(self->data);
	}
	else
	{
		memset(newData, 0, newCapacity * self->rowSize);
	}
	self->data     = newData;
	self->capacity = newCapacity;
	return true;
}

void* mdynarray_begin(mdynarray_t* self)
{
	return self ? self->data : NULL;
}

void* mdynarray_end(mdynarray_t* self)
{
	return self ? ((uint8_t*) self->data + self->size * self->rowSize) : NULL;
}

void* mdynarray_get(mdynarray_t* self, size_t index)
{
	return self && index < self->capacity ? ((uint8_t*) self->data + index * self->rowSize) : NULL;
}

bool mdynarray_set(mdynarray_t* self, size_t index, const void* data)
{
	if (!self || !data)
		return false;
	void* ptr = mdynarray_get(self, index);
	if (!ptr)
		return false;
	memcpy(ptr, data, self->rowSize);
	return true;
}

bool mdynarray_pushback(mdynarray_t* self, const void* data)
{
	if (!self || !data ||
		!mdynarray_reserve(self, self->size + 1))
		return false;

	if (!mdynarray_set(self, self->size, data))
		return false;
	++self->size;
	return true;
}

bool mdynarray_pushfront(mdynarray_t* self, const void* data)
{
	if (!self || !data ||
		!mdynarray_reserve(self, self->size + 1))
		return false;

	memmove((uint8_t*) self->data + self->rowSize, self->data, self->size * self->rowSize);
	mdynarray_set(self, 0, data);
	++self->size;
	return true;
}

bool mdynarray_insert(mdynarray_t* self, size_t index, const void* data)
{
	if (!self || !data ||
		!mdynarray_reserve(self, self->size + 1))
		return false;

	if (index >= self->size)
		index = self->size;
	memmove((uint8_t*) self->data + (index + 1) * self->rowSize, (uint8_t*) self->data + index * self->rowSize, (self->size - index) * self->rowSize);
	mdynarray_set(self, index, data);
	++self->size;
	return true;
}

bool mdynarray_popback(mdynarray_t* self)
{
	if (!self)
		return false;

	--self->size;
	memset((uint8_t*) self->data + self->size * self->rowSize, 0, self->rowSize);
	return true;
}

bool mdynarray_popfront(mdynarray_t* self)
{
	if (!self)
		return false;

	--self->size;
	memmove(self->data, (uint8_t*) self->data + self->rowSize, self->size * self->rowSize);
	memset((uint8_t*) self->data + self->size * self->rowSize, 0, self->rowSize);
	return true;
}

bool mdynarray_erase(mdynarray_t* self, size_t index, size_t length)
{
	if (!self)
		return false;
	if (index >= self->size)
		index = self->size;
	length = min(length, self->size - index);
	if (length == 0)
		return true;

	size_t end    = index + length;
	size_t toMove = self->size - end;
	memmove((uint8_t*) self->data + index * self->rowSize, (uint8_t*) self->data + end * self->rowSize, toMove * self->rowSize);
	memset((uint8_t*) self->data + (index + toMove) * self->rowSize, 0, length * self->rowSize);
	self->size -= length;
	return true;
}

mmultidynarray_t mmultidynarray(size_t initialCapacity, const size_t* initialSizes, size_t initialColumns)
{
	mmultidynarray_t arr;
	memset(&arr, 0, sizeof(arr));
	mmultidynarray_cstr(&arr, initialCapacity, initialSizes, initialColumns);
	return arr;
}

mmultidynarray_t* mmultidynarray_new(size_t initialCapacity, const size_t* initialSizes, size_t initialColumns)
{
	mmultidynarray_t* arr = (mmultidynarray_t*) mmalloc(sizeof(mmultidynarray_t));
	mmultidynarray_cstr(arr, initialCapacity, initialSizes, initialColumns);
	return arr;
}

void mmultidynarray_del(mmultidynarray_t* self)
{
	if (!self)
		return;
	mmultidynarray_dstr(self);
	mfree(self);
}

bool mmultidynarray_cstr(mmultidynarray_t* self, size_t initialCapacity, const size_t* initialSizes, size_t initialColumns)
{
	if (!self || !initialSizes || !initialColumns)
		return false;
	self->data     = NULL;
	self->size     = 0;
	self->capacity = 0;
	self->columns  = initialColumns;
	self->sizes    = (size_t*) mmalloc(self->columns * sizeof(size_t));
	if (!self->sizes)
	{
		self->columns = 0;
		return false;
	}
	memcpy(self->sizes, initialSizes, self->columns * sizeof(size_t));
	return mmultidynarray_reserve(self, initialCapacity);
}

void mmultidynarray_dstr(mmultidynarray_t* self)
{
	if (!self)
		return;
	mfree(self->data);
	mfree(self->sizes);
	self->data     = NULL;
	self->size     = 0;
	self->capacity = 0;
	self->columns  = 0;
	self->sizes    = NULL;
}

bool mmultidynarray_copy(mmultidynarray_t* self, const mmultidynarray_t* copy)
{
	if (!self || !copy)
		return false;
	mmultidynarray_dstr(self);
	if (!mmultidynarray_cstr(self, copy->size, copy->sizes, copy->columns))
		return false;
	size_t offset1 = 0;
	size_t offset2 = 0;
	for (size_t i = 0; i < self->columns; ++i)
	{
		size_t rowSize = self->sizes[i];
		memcpy((uint8_t*) self->data + offset1, (uint8_t*) copy->data + offset2, self->size * rowSize);
		offset1 += self->capacity * rowSize;
		offset2 += copy->capacity * rowSize;
	}
	self->size = copy->size;
	return true;
}

bool mmultidynarray_clear(mmultidynarray_t* self)
{
	if (!self)
		return false;

	if (self->data)
	{
		for (size_t i = 0; i < self->columns; ++i)
			memset(mmultidynarray_get(self, 0, i), 0, self->sizes[i] * self->size);
	}
	self->size = 0;
	return true;
}

bool mmultidynarray_resize(mmultidynarray_t* self, size_t newSize)
{
	if (!self ||
		!mmultidynarray_reserve(self, newSize))
		return false;

	if (self->size < newSize)
	{
		for (size_t i = 0; i < self->columns; ++i)
			memset(mmultidynarray_get(self, newSize, i), 0, self->size - newSize);
	}
	self->size = newSize;
	return true;
}

bool mmultidynarray_reserve(mmultidynarray_t* self, size_t newCapacity)
{
	if (!self)
		return false;
	newCapacity = mbit_ceilfull(newCapacity);
	if (newCapacity <= self->capacity)
		return true;

	size_t totalSize = 0;
	for (size_t i = 0; i < self->columns; ++i)
		totalSize += newCapacity * malign_ceil(self->sizes[i], 16);

	void* newData = mmalloc(totalSize);
	if (!newData)
		return false;
	if (self->data)
	{
		size_t offset1 = 0, offset2 = 0;
		for (size_t i = 0; i < self->columns; ++i)
		{
			size_t rowSize = self->sizes[i];
			memcpy((uint8_t*) newData + offset2, (uint8_t*) self->data + offset1, self->size * rowSize);
			memset((uint8_t*) newData + (self->size + 1) * rowSize, 0, (newCapacity - self->size) * rowSize);
			offset1 += self->capacity * rowSize;
			offset2 += newCapacity * rowSize;
		}
		mfree(self->data);
	}
	else
	{
		memset(newData, 0, totalSize);
	}
	self->data     = newData;
	self->capacity = newCapacity;
	return true;
}

bool mmultidynarray_pushback_column(mmultidynarray_t* self, size_t size)
{
	if (!self || !size)
		return false;
	size_t* newSizes = (size_t*) mmalloc((self->columns + 1) * sizeof(size_t));
	if (!newSizes)
		return false;

	size_t totalSize = self->capacity * malign_ceil(size, 16);
	for (size_t i = 0; i < self->columns; ++i)
		totalSize += self->capacity * malign_ceil(self->sizes[i], 16);
	void* newData = mmalloc(totalSize);
	if (!newData)
	{
		mfree(newSizes);
		return false;
	}
	memcpy(newSizes, self->sizes, self->columns * sizeof(size_t));
	newSizes[self->columns] = size;
	if (self->data)
	{
		size_t offset = 0;
		for (size_t i = 0; i < self->columns; ++i)
		{
			size_t rowSize = newSizes[i];
			memcpy((uint8_t*) newData + offset, (uint8_t*) self->data + offset, self->capacity * rowSize);
			offset += self->capacity * rowSize;
		}
		memset((uint8_t*) newData + offset, 0, self->capacity * newSizes[self->columns]);
		mfree(self->data);
	}
	else
	{
		memset(newData, 0, totalSize);
	}
	self->data  = newData;
	self->sizes = newSizes;
	++self->columns;
	return true;
}

bool mmultidynarray_pushfront_column(mmultidynarray_t* self, size_t size)
{
	if (!self || !size)
		return false;
	size_t* newSizes = (size_t*) mmalloc((self->columns + 1) * sizeof(size_t));
	if (!newSizes)
		return false;

	size_t totalSize = self->capacity * malign_ceil(size, 16);
	for (size_t i = 0; i < self->columns; ++i)
		totalSize += self->capacity * malign_ceil(self->sizes[i], 16);
	void* newData = mmalloc(totalSize);
	if (!newData)
	{
		mfree(newSizes);
		return false;
	}
	memcpy(newSizes + sizeof(size_t), self->sizes, self->columns * sizeof(size_t));
	newSizes[0] = size;
	if (self->data)
	{
		size_t offset1 = self->capacity * newSizes[0], offset2 = 0;
		for (size_t i = 0; i < self->columns; ++i)
		{
			size_t rowSize = newSizes[i + 1];
			memcpy((uint8_t*) newData + offset1, (uint8_t*) self->data + offset2, self->capacity * rowSize);
			offset1 += self->capacity * rowSize;
			offset2 += self->capacity * rowSize;
		}
		memset((uint8_t*) newData, 0, self->capacity * newSizes[0]);
		mfree(self->data);
	}
	else
	{
		memset(newData, 0, totalSize);
	}
	self->data  = newData;
	self->sizes = newSizes;
	++self->columns;
	return true;
}

// TODO(MarcasRealAccount): Implement mmultidynarray_popback_column, mmultidynarray_popfront_column, mmultidynarray_insert_column and mmultidynarray_erase_columns
bool mmultidynarray_popback_column(mmultidynarray_t* self)
{
	(void) self;
	return false;
}

bool mmultidynarray_popfront_column(mmultidynarray_t* self)
{
	(void) self;
	return false;
}

bool mmultidynarray_insert_column(mmultidynarray_t* self, size_t column, size_t size)
{
	(void) self;
	(void) column;
	(void) size;
	return false;
}

bool mmultidynarray_erase_columns(mmultidynarray_t* self, size_t column, size_t length)
{
	(void) self;
	(void) column;
	(void) length;
	return false;
}

bool mmultidynarray_shrink_to_fit(mmultidynarray_t* self)
{
	if (!self)
		return false;
	size_t newCapacity = mbit_ceilfull(self->size);
	if (newCapacity <= self->capacity)
		return true;

	size_t totalSize = 0;
	for (size_t i = 0; i < self->columns; ++i)
		totalSize += newCapacity * malign_ceil(self->sizes[i], 16);

	void* newData = mmalloc(totalSize);
	if (!newData)
		return false;
	if (self->data)
	{
		size_t offset1 = 0, offset2 = 0;
		for (size_t i = 0; i < self->columns; ++i)
		{
			size_t rowSize = self->sizes[i];
			memcpy((uint8_t*) newData + offset2, (uint8_t*) self->data + offset1, self->size * rowSize);
			memset((uint8_t*) newData + (self->size + 1) * rowSize, 0, (newCapacity - self->size) * rowSize);
			offset1 += self->capacity * rowSize;
			offset2 += newCapacity * rowSize;
		}
		mfree(self->data);
	}
	else
	{
		memset(newData, 0, totalSize);
	}
	self->data     = newData;
	self->capacity = newCapacity;
	return true;
}

mmultidynarrayrow_t mmultidynarray_begin(mmultidynarray_t* self)
{
	mmultidynarrayrow_t row = {
		.arr = self,
		.row = 0
	};
	return row;
}

mmultidynarrayrow_t mmultidynarray_end(mmultidynarray_t* self)
{
	mmultidynarrayrow_t row = {
		.arr = self,
		.row = self ? self->size : 0
	};
	return row;
}

mmultidynarrayrow_t mmultidynarray_get_row(mmultidynarray_t* self, size_t row)
{
	mmultidynarrayrow_t out = {
		.arr = self,
		.row = self ? min(row, self->size) : 0
	};
	return out;
}

bool mmultidynarray_set_row(mmultidynarray_t* self, size_t row, const void** data)
{
	if (!self || row >= self->size)
		return false;

	for (size_t i = 0; i < self->columns; ++i)
		memcpy(mmultidynarray_get(self, row, i), data[i], self->sizes[i]);
	return true;
}

void* mmultidynarray_get(mmultidynarray_t* self, size_t row, size_t column)
{
	if (!self || row >= self->size || column >= self->columns)
		return NULL;

	size_t offset = 0;
	for (size_t i = 0; i < column; ++i)
		offset += self->capacity * malign_ceil(self->sizes[i], 16);
	return (uint8_t*) self->data + offset + row * self->sizes[column];
}

bool mmultidynarray_set(mmultidynarray_t* self, size_t row, size_t column, const void* data)
{
	if (!self || row >= self->size || column >= self->columns)
		return false;
	void* ptr = mmultidynarray_get(self, row, column);
	memcpy(ptr, data, self->sizes[column]);
	return true;
}

bool mmultidynarray_pushback(mmultidynarray_t* self, const void** data)
{
	if (!self || !data ||
		!mmultidynarray_reserve(self, self->size + 1))
		return false;

	if (!mmultidynarray_set_row(self, self->size, data))
		return false;
	++self->size;
	return true;
}

bool mmultidynarray_pushfront(mmultidynarray_t* self, const void** data)
{
	if (!self || !data ||
		!mmultidynarray_reserve(self, self->size + 1))
		return false;

	size_t offset = 0;
	for (size_t i = 0; i < self->columns; ++i)
	{
		size_t rowSize = self->sizes[i];
		memmove((uint8_t*) self->data + offset + rowSize, (uint8_t*) self->data + offset, self->size * rowSize);
		offset += self->capacity * malign_ceil(rowSize, 16);
	}
	mmultidynarray_set_row(self, 0, data);
	++self->size;
	return true;
}

bool mmultidynarray_insert(mmultidynarray_t* self, size_t row, const void** data)
{
	if (!self || !data ||
		!mmultidynarray_reserve(self, self->size + 1))
		return false;

	if (row >= self->size)
		row = self->size;

	size_t offset = 0;
	for (size_t i = 0; i < self->columns; ++i)
	{
		size_t rowSize = self->sizes[i];
		memmove((uint8_t*) self->data + offset + (row + 1) * rowSize, (uint8_t*) self->data + offset + row * rowSize, (self->size - row) * rowSize);
		offset += self->capacity * malign_ceil(rowSize, 16);
	}
	mmultidynarray_set_row(self, row, data);
	++self->size;
	return true;
}

bool mmultidynarray_popback(mmultidynarray_t* self)
{
	if (!self)
		return false;

	--self->size;
	size_t offset = 0;
	for (size_t i = 0; i < self->columns; ++i)
	{
		size_t rowSize = self->sizes[i];
		memset((uint8_t*) self->data + offset + self->size * rowSize, 0, rowSize);
		offset += self->capacity * malign_ceil(rowSize, 16);
	}
	return true;
}

bool mmultidynarray_popfront(mmultidynarray_t* self)
{
	if (!self)
		return false;

	--self->size;
	size_t offset = 0;
	for (size_t i = 0; i < self->columns; ++i)
	{
		size_t rowSize = self->sizes[i];
		memmove((uint8_t*) self->data + offset, (uint8_t*) self->data + offset + rowSize, self->size * rowSize);
		memset((uint8_t*) self->data + offset + self->size * rowSize, 0, rowSize);
		offset += self->capacity * malign_ceil(rowSize, 16);
	}
	return true;
}

bool mmultidynarray_erase(mmultidynarray_t* self, size_t row, size_t length)
{
	if (!self)
		return false;
	if (row >= self->size)
		row = self->size;
	length = min(length, self->size - row);

	size_t end    = row + length;
	size_t toMove = self->size - end;
	size_t offset = 0;
	for (size_t i = 0; i < self->columns; ++i)
	{
		size_t rowSize = self->sizes[i];
		memmove((uint8_t*) self->data + offset + row * rowSize, (uint8_t*) self->data + end * rowSize, toMove * rowSize);
		memset((uint8_t*) self->data + (row + toMove) * rowSize, 0, length * rowSize);
		offset += self->capacity * malign_ceil(rowSize, 16);
	}
	self->size -= length;
	return true;
}

int mmultidynarrayrow_cmp(mmultidynarrayrow_t lhs, mmultidynarrayrow_t rhs)
{
	if (lhs.arr < rhs.arr)
		return -1;
	if (lhs.arr > rhs.arr)
		return 1;
	if (lhs.row < rhs.row)
		return -1;
	if (lhs.row > rhs.row)
		return 1;
	return 0;
}

bool mmultidynarrayrow_iter(mmultidynarrayrow_t* row)
{
	if (!row || !row->arr)
		return false;
	++row->row;
	return true;
}

void* mmultidynarrayrow_get(mmultidynarrayrow_t row, size_t column)
{
	return mmultidynarray_get(row.arr, row.row, column);
}

bool mmultidynarrayrow_set(mmultidynarrayrow_t row, size_t column, const void* data)
{
	return mmultidynarray_set(row.arr, row.row, column, data);
}