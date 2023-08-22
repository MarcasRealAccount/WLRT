#include "str.h"
#include "mem.h"

#include <stdlib.h>
#include <string.h>

mstring_t mstringc(char c, size_t len)
{
	mstring_t out = mstring(len);
	mstring_fill(&out, 0, len, c);
	out.length = len;
	return out;
}

mstring_t mstringcstr(const char* str)
{
	return mstringv(mstringviewcstr(str));
}

mstring_t mstringv(mstringview_t view)
{
	mstring_t out;
	memset(&out, 0, sizeof(out));
	mstring_assign(&out, view);
	return out;
}

mstring_t mstring(size_t initialCapacity)
{
	mstring_t out;
	memset(&out, 0, sizeof(out));
	mstring_cstr(&out, initialCapacity);
	return out;
}

mstring_t* mstring_new(size_t initialCapacity)
{
	mstring_t* out = (mstring_t*) mmalloc_aligned(sizeof(mstring_t), alignof(mstring_t));
	if (!out)
		return NULL;
	mstring_cstr(out, initialCapacity);
	return out;
}

void mstring_del(mstring_t* self)
{
	if (!self)
		return;
	mstring_dstr(self);
	mfree_aligned(self, alignof(mstring_t));
}

bool mstring_cstr(mstring_t* self, size_t initialCapacity)
{
	if (!self)
		return false;
	self->length   = 0;
	self->capacity = 0;
	memset(self->sso, 0, sizeof(self->sso));
	return mstring_reserve(self, initialCapacity);
}

void mstring_dstr(mstring_t* self)
{
	if (!self)
		return;
	if (self->capacity > sizeof(self->sso) - 1)
		mfree(*(char**) self->sso);
	self->length   = 0;
	self->capacity = 0;
	memset(self->sso, 0, sizeof(self->sso));
}

bool mstring_clear(mstring_t* self)
{
	if (!self)
		return false;
	memset(mstring_begin(self), 0, self->length * sizeof(char));
	self->length = 0;
	return true;
}

bool mstring_resize(mstring_t* self, size_t newLength)
{
	if (!self ||
		!mstring_reserve(self, newLength))
		return false;
	if (newLength < self->length)
		memset(mstring_begin(self) + newLength, 0, (self->length - newLength) * sizeof(char));
	self->length = newLength;
	return true;
}

bool mstring_reserve(mstring_t* self, size_t newCapacity)
{
	if (!self)
		return false;
	newCapacity = mbit_ceilfull(newCapacity);
	if (newCapacity < self->capacity)
		return true;
	if (newCapacity > sizeof(self->sso) - 1)
	{
		char* newData = (char*) mmalloc(newCapacity * sizeof(char));
		if (!newData)
			return false;
		memcpy(newData, mstring_begin(self), self->length * sizeof(char));
		memset(newData + self->length, 0, (newCapacity - self->length) * sizeof(char));
		if (self->capacity > sizeof(self->sso) - 1)
			mfree(*(char**) self->sso);
		memset(self->sso, 0, sizeof(self->sso));
		*(char**) self->sso = newData;
		self->capacity      = newCapacity;
	}
	else
	{
		self->capacity = 15;
	}
	return true;
}

bool mstring_shrink_to_fit(mstring_t* self)
{
	if (!self)
		return false;
	size_t newCapacity = mbit_ceilfull(self->length);
	if (newCapacity < self->capacity)
		return true;
	if (newCapacity > sizeof(self->sso) - 1)
	{
		char* newData = (char*) mmalloc(newCapacity * sizeof(char));
		if (!newData)
			return false;
		memcpy(newData, mstring_begin(self), self->length * sizeof(char));
		memset(newData + self->length, 0, (newCapacity - self->length) * sizeof(char));
		if (self->capacity > sizeof(self->sso) - 1)
			mfree(*(char**) self->sso);
		memset(self->sso, 0, sizeof(self->sso));
		*(char**) self->sso = newData;
		self->capacity      = newCapacity;
	}
	else
	{
		self->capacity = 15;
	}
	return true;
}

char* mstring_begin(mstring_t* self)
{
	return self ? (self->capacity < sizeof(self->sso) ? self->sso : *(char**) self->sso) : NULL;
}

const char* mstring_cbegin(const mstring_t* self)
{
	return self ? (self->capacity < sizeof(self->sso) ? self->sso : *(char**) self->sso) : NULL;
}

char* mstring_end(mstring_t* self)
{
	return self ? (self->capacity < sizeof(self->sso) ? self->sso + 15 : *(char**) self->sso + self->length) : NULL;
}

const char* mstring_cend(const mstring_t* self)
{
	return self ? (self->capacity < sizeof(self->sso) ? self->sso + 15 : *(char**) self->sso + self->length) : NULL;
}

char* mstring_get(mstring_t* self, size_t index)
{
	return self && index < self->capacity ? (self->capacity < sizeof(self->sso) ? self->sso + index : *(char**) self->sso + index) : NULL;
}

const char* mstring_cget(const mstring_t* self, size_t index)
{
	return self && index < self->capacity ? (self->capacity < sizeof(self->sso) ? self->sso + index : *(char**) self->sso + index) : NULL;
}

bool mstring_set(mstring_t* self, size_t index, char c)
{
	if (!self)
		return false;
	char* ptr = mstring_get(self, index);
	if (!ptr)
		return false;
	*ptr = c;
	return true;
}

bool mstring_fill(mstring_t* self, size_t offset, size_t length, char c)
{
	if (!self)
		return false;
	if (offset >= self->capacity)
		return false;
	length = min(length, self->capacity - offset);
	memset(mstring_get(self, offset), c, length);
	return true;
}

mstringview_t mstring_substr(const mstring_t* self, size_t index, size_t length)
{
	return mstringview(mstring_cget(self, index), length);
}

bool mstring_pushback(mstring_t* self, char c)
{
	if (!self ||
		!mstring_reserve(self, self->length + 1))
		return false;
	mstring_set(self, self->length, c);
	++self->length;
	return true;
}

bool mstring_pushfront(mstring_t* self, char c)
{
	if (!self ||
		!mstring_reserve(self, self->length + 1))
		return false;
	char* begin = mstring_begin(self);
	memmove(begin + 1, begin, self->length * sizeof(char));
	mstring_set(self, 0, c);
	++self->length;
	return true;
}

bool mstring_insert(mstring_t* self, size_t index, char c)
{
	if (!self ||
		!mstring_reserve(self, self->length + 1))
		return false;
	char* begin = mstring_begin(self) + index;
	memmove(begin + 1, begin, (self->length - index) * sizeof(char));
	mstring_set(self, index, c);
	++self->length;
	return true;
}

bool mstring_popback(mstring_t* self)
{
	if (!self)
		return false;
	--self->length;
	memset(mstring_get(self, self->length), 0, sizeof(char));
	return true;
}

bool mstring_popfront(mstring_t* self)
{
	if (!self)
		return false;
	--self->length;
	char* begin = mstring_begin(self);
	memmove(begin, begin + 1, self->length * sizeof(char));
	memset(mstring_get(self, self->length), 0, sizeof(char));
	return true;
}

bool mstring_assign(mstring_t* self, mstringview_t string)
{
	if (!self ||
		!mstring_reserve(self, string.length))
		return false;
	char* begin = mstring_begin(self);
	memcpy(begin, string.string, string.length * sizeof(char));
	self->length        = string.length;
	begin[self->length] = '\0';
	return true;
}

bool mstring_append(mstring_t* self, mstringview_t string)
{
	if (!self ||
		!mstring_reserve(self, self->length + string.length))
		return false;
	char* begin = mstring_begin(self);
	memcpy(begin + self->length, string.string, string.length * sizeof(char));
	self->length       += string.length;
	begin[self->length] = '\0';
	return true;
}

bool mstring_prepend(mstring_t* self, mstringview_t string)
{
	if (!self ||
		!mstring_reserve(self, self->length + string.length))
		return false;
	char* begin = mstring_begin(self);
	memmove(begin + string.length, begin, self->length * sizeof(char));
	memcpy(begin, string.string, string.length * sizeof(char));
	self->length       += string.length;
	begin[self->length] = '\0';
	return true;
}

bool mstring_replace(mstring_t* self, size_t index, size_t length, mstringview_t replacement)
{
	if (!self)
		return false;

	if (index >= self->length)
		index = self->length;
	length              = min(length, self->length - index);
	size_t requiredSize = self->length - length + replacement.length;
	if (!mstring_reserve(self, requiredSize))
		return false;
	size_t end   = index + length;
	char*  begin = mstring_begin(self);
	memmove(begin + index + replacement.length, begin + end, (self->length - end) * sizeof(char));
	memcpy(begin + index, replacement.string, replacement.length * sizeof(char));
	self->length        = requiredSize;
	begin[self->length] = '\0';
	return true;
}

bool mstring_erase(mstring_t* self, size_t index, size_t length)
{
	if (!self)
		return false;
	if (index >= self->length)
		index = self->length;
	length = min(length, self->length - index);
	if (length == 0)
		return true;
	size_t end   = index + length;
	char*  begin = mstring_begin(self);
	memmove(begin + index, begin + end, (self->length - end) * sizeof(char));
	self->length       -= length;
	begin[self->length] = '\0';
	return true;
}

int mstring_compare(const mstring_t* lhs, mstringview_t rhs)
{
	return mstringview_compare(mstringviews(lhs), rhs);
}

size_t mstring_find(const mstring_t* self, mstringview_t substring, size_t offset)
{
	return mstringview_find(mstringviews(self), substring, offset);
}

size_t mstring_rfind(const mstring_t* self, mstringview_t substring, size_t offset)
{
	return mstringview_rfind(mstringviews(self), substring, offset);
}

size_t mstring_find_first_of(const mstring_t* self, mstringview_t chars, size_t offset)
{
	return mstringview_find_first_of(mstringviews(self), chars, offset);
}

size_t mstring_find_first_not_of(const mstring_t* self, mstringview_t chars, size_t offset)
{
	return mstringview_find_first_not_of(mstringviews(self), chars, offset);
}

size_t mstring_find_last_of(const mstring_t* self, mstringview_t chars, size_t offset)
{
	return mstringview_find_last_of(mstringviews(self), chars, offset);
}

size_t mstring_find_last_not_of(const mstring_t* self, mstringview_t chars, size_t offset)
{
	return mstringview_find_last_not_of(mstringviews(self), chars, offset);
}

mstringview_t mstringviewcstr(const char* str)
{
	mstringview_t out;
	memset(&out, 0, sizeof(out));
	mstringview_cstr(&out, str, str ? strlen(str) : 0);
	return out;
}

mstringview_t mstringviews(const mstring_t* string)
{
	mstringview_t out;
	memset(&out, 0, sizeof(out));
	mstringview_cstr(&out, mstring_cbegin(string), string ? string->length : 0);
	return out;
}

mstringview_t mstringview(const char* string, size_t length)
{
	mstringview_t out;
	memset(&out, 0, sizeof(out));
	mstringview_cstr(&out, string, length);
	return out;
}

bool mstringview_cstr(mstringview_t* self, const char* string, size_t length)
{
	if (!self)
		return false;
	if (!string)
	{
		self->string = string;
		self->length = 0;
	}
	else
	{
		self->string = string;
		self->length = length == ~0ULL ? strlen(string) : length;
	}
	return true;
}

void mstringview_dstr(mstringview_t* self)
{
	if (!self)
		return;

	self->string = NULL;
	self->length = 0;
}

const char* mstringview_begin(mstringview_t self)
{
	return self.string;
}

const char* mstringview_end(mstringview_t self)
{
	return self.string ? self.string + self.length : NULL;
}

const char* mstringview_get(mstringview_t self, size_t index)
{
	return self.string && index < self.length ? self.string + index : NULL;
}

mstringview_t mstringview_substr(mstringview_t self, size_t index, size_t length)
{
	if (!self.string)
		return mstringviewcstr(NULL);
	if (index >= self.length)
		index = self.length;
	length            = min(length, self.length - index);
	mstringview_t out = {
		.string = self.string + index,
		.length = length
	};
	return out;
}

int mstringview_compare(mstringview_t lhs, mstringview_t rhs)
{
	if (lhs.string < rhs.string)
		return -1;
	if (rhs.string > rhs.string)
		return 1;
	if (lhs.length < rhs.length)
		return -1;
	if (lhs.length > rhs.length)
		return 1;
	for (size_t i = 0; i < lhs.length; ++i)
	{
		char a = lhs.string[i];
		char b = rhs.string[i];
		if (a < b)
			return -1;
		if (a > b)
			return 1;
	}
	return 0;
}

size_t mstringview_find(mstringview_t self, mstringview_t substring, size_t offset)
{
	size_t i = offset, j = 0;
	for (; i < self.length - substring.length; ++i)
	{
		char a = self.string[i + j];
		char b = substring.string[j];
		if (a == b)
		{
			if (++j == substring.length)
				break;
		}
		else
		{
			j = 0;
		}
	}
	if (i >= self.length - substring.length)
		return self.length;
	return i;
}

size_t mstringview_rfind(mstringview_t self, mstringview_t substring, size_t offset)
{
	size_t i = self.length - offset - 1, j = substring.length;
	for (; i >= substring.length; --i)
	{
		char a = self.string[i - j];
		char b = substring.string[substring.length - j];
		if (a == b)
		{
			if (--j == 0)
				break;
		}
		else
		{
			j = substring.length;
		}
	}
	if (i < substring.length)
		return self.length;
	return i;
}

size_t mstringview_find_first_of(mstringview_t self, mstringview_t chars, size_t offset)
{
	size_t i     = offset;
	bool   found = false;
	for (; i < self.length; ++i)
	{
		char a = self.string[i];
		for (size_t j = 0; !found && j < chars.length; ++j)
		{
			if (chars.string[j] == a)
				found = true;
		}
		if (found)
			break;
	}
	if (i >= self.length)
		return self.length;
	return i;
}

size_t mstringview_find_first_not_of(mstringview_t self, mstringview_t chars, size_t offset)
{
	size_t i     = offset;
	bool   found = false;
	for (; i < self.length; ++i)
	{
		char a = self.string[i];
		for (size_t j = 0; !found && j < chars.length; ++j)
		{
			if (chars.string[j] == a)
				found = true;
		}
		if (found)
			break;
	}
	if (i >= self.length)
		return self.length;
	return i;
}

size_t mstringview_find_last_of(mstringview_t self, mstringview_t chars, size_t offset)
{
	size_t i     = self.length - offset - 1;
	bool   found = false;
	for (; i != ~0ULL; --i)
	{
		char a = self.string[i];
		for (size_t j = 0; !found && j < chars.length; ++j)
		{
			if (chars.string[j] == a)
				found = true;
		}
		if (found)
			break;
	}
	return min(i, self.length);
}

size_t mstringview_find_last_not_of(mstringview_t self, mstringview_t chars, size_t offset)
{
	size_t i     = self.length - offset - 1;
	bool   found = false;
	for (; i != ~0ULL; --i)
	{
		char a = self.string[i];
		for (size_t j = 0; !found && j < chars.length; ++j)
		{
			if (chars.string[j] == a)
				found = true;
		}
		if (found)
			break;
	}
	return min(i, self.length);
}

mstring_t mstring_format(mstringview_t format, ...)
{
	va_list args;
	va_start(args, format);
	mstring_t result = mstring_format_va(format, args);
	va_end(args);
	return result;
}

mstring_t mstring_format_va(mstringview_t format, va_list args)
{
	if (!format.string || !format.length)
		return mstring(0);
	mstring_t result = mstring(0);
	size_t    offset = 0;
	while (offset < format.length)
	{
		size_t start = mstringview_find_first_of(format, mstringviewcstr("%"), offset);
		size_t len   = start - offset;
		if (len > 0)
			mstring_append(&result, mstringview_substr(format, offset, len));
		if (start + 1 >= format.length)
			break;
		offset = start + 1;

		if (format.string[offset] == '%')
		{
			mstring_pushback(&result, '%');
			++offset;
			continue;
		}

		switch (format.string[offset])
		{
		case 'c':
			++offset;
			mstring_pushback(&result, va_arg(args, char));
			break;
		case 's':
			++offset;
			mstring_append(&result, mstringviewcstr(va_arg(args, const char*)));
			break;
		case 'b':
		{
			++offset;
			mstring_t str = mstring_from_uint(va_arg(args, uint64_t), 2);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'o':
		{
			++offset;
			mstring_t str = mstring_from_uint(va_arg(args, uint64_t), 7);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'd':
		{
			++offset;
			mstring_t str = mstring_from_int(va_arg(args, int64_t), 10);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'u':
		{
			++offset;
			mstring_t str = mstring_from_uint(va_arg(args, uint64_t), 10);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'x':
		{
			++offset;
			mstring_t str = mstring_from_uint(va_arg(args, int64_t), 16);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'p':
		{
			++offset;
			uint64_t  value = (uint64_t) va_arg(args, void*);
			mstring_t str   = mstring_from_uint(value, 16);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
		}
		case 'f':
		{
			++offset;
			mstring_t str = mstring_from_float(va_arg(args, double));
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'e':
		{
			++offset;
			mstring_t str = mstring_from_float_sci(va_arg(args, double), 10);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'a':
		{
			++offset;
			mstring_t str = mstring_from_float_sci(va_arg(args, double), 16);
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		case 'g':
		{
			++offset;
			mstring_t str = mstring_from_float_gen(va_arg(args, double));
			mstring_append(&result, mstringviews(&str));
			mstring_dstr(&str);
			break;
		}
		}
	}
	return result;
}

static char s_HexDigits[]    = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
static char s_Base32Digits[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '6', '7', '=' };
static char s_Base64Digits[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '=' };

static uint8_t s_HexIndices[]    = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t s_Base32Indices[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0x20, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t s_Base64Indices[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

mstring_t mstring_from_uint(uint64_t value, size_t radix)
{
	if (radix > 64)
		return mstringcstr(NULL);
	const char* digits;
	if (radix <= 16)
		digits = s_HexDigits;
	else if (radix <= 32)
		digits = s_Base32Digits;
	else
		digits = s_Base64Digits;
	char   buf[64];
	size_t i = 0;
	for (; i < 64; ++i)
	{
		buf[63 - i] = digits[value % radix];
		if ((value /= radix) == 0)
			break;
	}
	return mstringv(mstringview(buf + 63 - i, i + 1));
}

mstring_t mstring_from_int(int64_t value, size_t radix)
{
	bool      negative = value < 0;
	mstring_t out      = mstring_from_uint(negative ? -value : value, radix);
	if (negative)
		mstring_pushfront(&out, '-');
	return out;
}

// TODO(MarcasRealAccount): Implement mstring_from_float, mstring_from_float_sci and mstring_from_float_gen
mstring_t mstring_from_float(double value)
{
	(void) value;
	return mstring(0);
}

mstring_t mstring_from_float_sci(double value, size_t radix)
{
	(void) value;
	(void) radix;
	return mstring(0);
}

mstring_t mstring_from_float_gen(double value)
{
	(void) value;
	return mstring(0);
}

uint64_t mstring_to_uint(mstringview_t string, size_t radix, size_t* length)
{
	if (length)
		*length = 0;
	if (!string.string || !string.length || radix > 64)
		return 0;
	const uint8_t* indices;
	if (radix <= 16)
		indices = s_HexIndices;
	else if (radix <= 32)
		indices = s_Base32Indices;
	else
		indices = s_Base64Indices;
	size_t end = 0;
	while (end < string.length && indices[string.string[end]] != 0xFF)
		++end;

	size_t   multiplier = 1;
	uint64_t value      = 0;
	for (size_t i = 0; i < end; ++i)
	{
		value      += indices[string.string[end - i - 1]] * multiplier;
		multiplier *= radix;
	}
	if (length)
		*length = end;
	return value;
}

int64_t mstring_to_int(mstringview_t string, size_t radix, size_t* length)
{
	if (length)
		*length = 0;
	if (!string.string || !string.length || radix > 64)
		return 0;
	bool   negative = false;
	size_t offset   = 0;
	switch (string.string[0])
	{
	case '-':
		negative = true;
		offset   = 1;
		break;
	case '+':
		offset = 1;
		break;
	}
	uint64_t value = mstring_to_uint(mstringview_substr(string, offset, ~0ULL), radix, length);
	if (negative)
		return -(int64_t) value;
	else
		return (int64_t) value;
}

// TODO(MarcasRealAccount): Implement mstring_to_float, mstring_to_float_sci and mstring_to_float_gen
double mstring_to_float(mstringview_t string)
{
	(void) string;
	return 0.0;
}

double mstring_to_float_sci(mstringview_t string, size_t radix)
{
	(void) string;
	(void) radix;
	return 0.0;
}

double mstring_to_float_gen(mstringview_t string)
{
	(void) string;
	return 0.0;
}