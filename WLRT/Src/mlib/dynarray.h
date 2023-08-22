#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct mdynarray_t
{
	void*  data;
	size_t size;
	size_t capacity;
	size_t rowSize;
} mdynarray_t;

typedef struct mmultidynarray_t
{
	void*   data;
	size_t  size;
	size_t  capacity;
	size_t  columns;
	size_t* sizes;
} mmultidynarray_t;

typedef struct mmultidynarrayrow_t
{
	mmultidynarray_t* arr;
	size_t            row;
} mmultidynarrayrow_t;

mdynarray_t  mdynarray(size_t initialCapacity, size_t rowSize);
mdynarray_t* mdynarray_new(size_t initialCapacity, size_t rowSize);
void         mdynarray_del(mdynarray_t* self);
bool         mdynarray_cstr(mdynarray_t* self, size_t initialCapacity, size_t rowSize);
void         mdynarray_dstr(mdynarray_t* self);
bool         mdynarray_copy(mdynarray_t* self, const mdynarray_t* copy);
bool         mdynarray_clear(mdynarray_t* self);
bool         mdynarray_resize(mdynarray_t* self, size_t newSize);
bool         mdynarray_reserve(mdynarray_t* self, size_t newCapacity);
bool         mdynarray_shrink_to_fit(mdynarray_t* self);
void*        mdynarray_begin(mdynarray_t* self);
void*        mdynarray_end(mdynarray_t* self);
void*        mdynarray_get(mdynarray_t* self, size_t index);
bool         mdynarray_set(mdynarray_t* self, size_t index, const void* data);
bool         mdynarray_pushback(mdynarray_t* self, const void* data);
bool         mdynarray_pushfront(mdynarray_t* self, const void* data);
bool         mdynarray_insert(mdynarray_t* self, size_t index, const void* data);
bool         mdynarray_popback(mdynarray_t* self);
bool         mdynarray_popfront(mdynarray_t* self);
bool         mdynarray_erase(mdynarray_t* self, size_t index, size_t length);

mmultidynarray_t    mmultidynarray(size_t initialCapacity, const size_t* initialSizes, size_t initialColumns);
mmultidynarray_t*   mmultidynarray_new(size_t initialCapacity, const size_t* initialSizes, size_t initialColumns);
void                mmultidynarray_del(mmultidynarray_t* self);
bool                mmultidynarray_cstr(mmultidynarray_t* self, size_t initialCapacity, const size_t* initialSizes, size_t initialColumns);
void                mmultidynarray_dstr(mmultidynarray_t* self);
bool                mmultidynarray_copy(mmultidynarray_t* self, const mmultidynarray_t* copy);
bool                mmultidynarray_clear(mmultidynarray_t* self);
bool                mmultidynarray_resize(mmultidynarray_t* self, size_t newSize);
bool                mmultidynarray_reserve(mmultidynarray_t* self, size_t newCapacity);
bool                mmultidynarray_pushback_column(mmultidynarray_t* self, size_t size);
bool                mmultidynarray_pushfront_column(mmultidynarray_t* self, size_t size);
bool                mmultidynarray_popback_column(mmultidynarray_t* self);
bool                mmultidynarray_popfront_column(mmultidynarray_t* self);
bool                mmultidynarray_insert_column(mmultidynarray_t* self, size_t column, size_t size);
bool                mmultidynarray_erase_columns(mmultidynarray_t* self, size_t column, size_t length);
bool                mmultidynarray_shrink_to_fit(mmultidynarray_t* self);
mmultidynarrayrow_t mmultidynarray_begin(mmultidynarray_t* self);
mmultidynarrayrow_t mmultidynarray_end(mmultidynarray_t* self);
mmultidynarrayrow_t mmultidynarray_get_row(mmultidynarray_t* self, size_t row);
bool                mmultidynarray_set_row(mmultidynarray_t* self, size_t row, const void** data);
void*               mmultidynarray_get(mmultidynarray_t* self, size_t row, size_t column);
bool                mmultidynarray_set(mmultidynarray_t* self, size_t row, size_t column, const void* data);
bool                mmultidynarray_pushback(mmultidynarray_t* self, const void** data);
bool                mmultidynarray_pushfront(mmultidynarray_t* self, const void** data);
bool                mmultidynarray_insert(mmultidynarray_t* self, size_t row, const void** data);
bool                mmultidynarray_popback(mmultidynarray_t* self);
bool                mmultidynarray_popfront(mmultidynarray_t* self);
bool                mmultidynarray_erase(mmultidynarray_t* self, size_t row, size_t length);

int   mmultidynarrayrow_cmp(mmultidynarrayrow_t lhs, mmultidynarrayrow_t rhs);
bool  mmultidynarrayrow_iter(mmultidynarrayrow_t* row);
void* mmultidynarrayrow_get(mmultidynarrayrow_t row, size_t column);
bool  mmultidynarrayrow_set(mmultidynarrayrow_t row, size_t column, const void* data);