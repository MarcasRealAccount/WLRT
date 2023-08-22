#pragma once

#include "mem.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum mstring_e
{
	mstring_sso_min_num_chars_c = 16
} mstring_e;

typedef struct mstring_t
{
	alignas(64) size_t length;
	size_t capacity;
	char   sso[(mstring_sso_min_num_chars_c + 64) / 64 * 64 - 16];
} mstring_t;

typedef struct mstringview_t
{
	const char* string;
	size_t      length;
} mstringview_t;

mstring_t     mstringc(char c, size_t len);
mstring_t     mstringcstr(const char* str);
mstring_t     mstringv(mstringview_t view);
mstring_t     mstring(size_t initialCapacity);
mstring_t*    mstring_new(size_t initialCapacity);
void          mstring_del(mstring_t* self);
bool          mstring_cstr(mstring_t* self, size_t initialCapacity);
void          mstring_dstr(mstring_t* self);
bool          mstring_clear(mstring_t* self);
bool          mstring_resize(mstring_t* self, size_t newLength);
bool          mstring_reserve(mstring_t* self, size_t newCapacity);
bool          mstring_shrink_to_fit(mstring_t* self);
char*         mstring_begin(mstring_t* self);
const char*   mstring_cbegin(const mstring_t* self);
char*         mstring_end(mstring_t* self);
const char*   mstring_cend(const mstring_t* self);
char*         mstring_get(mstring_t* self, size_t index);
const char*   mstring_cget(const mstring_t* self, size_t index);
bool          mstring_set(mstring_t* self, size_t index, char c);
bool          mstring_fill(mstring_t* self, size_t offset, size_t length, char c);
mstringview_t mstring_substr(const mstring_t* self, size_t index, size_t length);
bool          mstring_pushback(mstring_t* self, char c);
bool          mstring_pushfront(mstring_t* self, char c);
bool          mstring_insert(mstring_t* self, size_t index, char c);
bool          mstring_popback(mstring_t* self);
bool          mstring_popfront(mstring_t* self);
bool          mstring_assign(mstring_t* self, mstringview_t string);
bool          mstring_append(mstring_t* self, mstringview_t string);
bool          mstring_prepend(mstring_t* self, mstringview_t string);
bool          mstring_replace(mstring_t* self, size_t index, size_t length, mstringview_t replacement);
bool          mstring_erase(mstring_t* self, size_t index, size_t length);
int           mstring_compare(const mstring_t* lhs, mstringview_t rhs);
size_t        mstring_find(const mstring_t* self, mstringview_t substring, size_t offset);
size_t        mstring_rfind(const mstring_t* self, mstringview_t substring, size_t offset);
size_t        mstring_find_first_of(const mstring_t* self, mstringview_t chars, size_t offset);
size_t        mstring_find_first_not_of(const mstring_t* self, mstringview_t chars, size_t offset);
size_t        mstring_find_last_of(const mstring_t* self, mstringview_t chars, size_t offset);
size_t        mstring_find_last_not_of(const mstring_t* self, mstringview_t chars, size_t offset);

mstringview_t mstringviewcstr(const char* str);
mstringview_t mstringviews(const mstring_t* string);
mstringview_t mstringview(const char* string, size_t length);
bool          mstringview_cstr(mstringview_t* self, const char* string, size_t length);
void          mstringview_dstr(mstringview_t* self);
const char*   mstringview_begin(mstringview_t self);
const char*   mstringview_end(mstringview_t self);
const char*   mstringview_get(mstringview_t self, size_t index);
mstringview_t mstringview_substr(mstringview_t self, size_t index, size_t length);
int           mstringview_compare(mstringview_t lhs, mstringview_t rhs);
size_t        mstringview_find(mstringview_t self, mstringview_t substring, size_t offset);
size_t        mstringview_rfind(mstringview_t self, mstringview_t substring, size_t offset);
size_t        mstringview_find_first_of(mstringview_t self, mstringview_t chars, size_t offset);
size_t        mstringview_find_first_not_of(mstringview_t self, mstringview_t chars, size_t offset);
size_t        mstringview_find_last_of(mstringview_t self, mstringview_t chars, size_t offset);
size_t        mstringview_find_last_not_of(mstringview_t self, mstringview_t chars, size_t offset);

mstring_t mstring_format(mstringview_t format, ...);
mstring_t mstring_format_va(mstringview_t format, va_list args);

mstring_t mstring_from_uint(uint64_t value, size_t radix);
mstring_t mstring_from_int(int64_t value, size_t radix);
mstring_t mstring_from_float(double value);
mstring_t mstring_from_float_sci(double value, size_t radix);
mstring_t mstring_from_float_gen(double value);
uint64_t  mstring_to_uint(mstringview_t string, size_t radix, size_t* length);
int64_t   mstring_to_int(mstringview_t string, size_t radix, size_t* length);
double    mstring_to_float(mstringview_t string);
double    mstring_to_float_sci(mstringview_t string, size_t radix);
double    mstring_to_float_gen(mstringview_t string);