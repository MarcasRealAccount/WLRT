#include "Str.h"
#include "Memory.h"

#include <stdio.h>
#include <string.h>

static bool WLRTStringEnsureCapacity(WLRTString* string, size_t capacity)
{
	if (capacity < string->capacity)
		return true;

	size_t newCapacity = capacity;
	--newCapacity;
	newCapacity |= newCapacity >> 1;
	newCapacity |= newCapacity >> 2;
	newCapacity |= newCapacity >> 4;
	newCapacity |= newCapacity >> 8;
	newCapacity |= newCapacity >> 16;
	newCapacity |= newCapacity >> 32;
	++newCapacity;

	if (newCapacity == string->capacity)
		return true;

	if (newCapacity > sizeof(string->sso) - 1)
	{
		void* newData = WLRTAlloc(newCapacity, alignof(char));
		if (!newData)
			return false;
		char* data = WLRTStringData(string);
		memcpy(newData, data, string->length);
		if (string->capacity > sizeof(string->sso) - 1)
			WLRTFree(data, alignof(char));
		memset(string->sso, 0, sizeof(string->sso));
		*(char**) string->sso = newData;
		string->capacity      = newCapacity;
	}
	else
	{
		string->capacity = 15;
	}
	return true;
}

bool WLRTStringSetup(WLRTString* string, size_t initialCapacity)
{
	string->length   = 0;
	string->capacity = 0;
	memset(string->sso, 0, sizeof(string->sso));
	if (!WLRTStringEnsureCapacity(string, initialCapacity))
	{
		WLRTStringCleanup(string);
		return false;
	}
	return true;
}

void WLRTStringCleanup(WLRTString* string)
{
	if (string->capacity > sizeof(string->sso) - 1)
		WLRTFree(*(void**) string->sso, alignof(char));
	memset(string->sso, 0, sizeof(string->sso));
	string->length   = 0;
	string->capacity = 0;
}

WLRTString WLRTStringCreate(const char* string, size_t length)
{
	if (length == ~0ULL)
		length = strlen(string);

	WLRTString result;
	if (!WLRTStringSetup(&result, length))
		return result;
	char* data = WLRTStringData(&result);
	memcpy(data, string, length);
	data[length]  = '\0';
	result.length = length;
	return result;
}

WLRTString WLRTStringCopy(WLRTStringView string)
{
	WLRTString result;
	if (!WLRTStringSetup(&result, string.length))
		return result;
	char* data = WLRTStringData(&result);
	memcpy(data, string.string, string.length);
	data[string.length] = '\0';
	result.length       = string.length;
	return result;
}

char* WLRTStringData(WLRTString* string)
{
	if (!string)
		return NULL;
	return (string->capacity > sizeof(string->sso) - 1) ? *(char**) string->sso : string->sso;
}

const char* WLRTStringCData(const WLRTString* string)
{
	if (!string)
		return NULL;
	return (string->capacity > sizeof(string->sso) - 1) ? *(const char**) string->sso : string->sso;
}

WLRTStringView WLRTStringSubstr(const WLRTString* string, size_t offset, size_t end)
{
	if (!string)
		return WLRTStringViewCreate(NULL, 0);
	if (offset >= string->length)
		offset = string->length;
	if (end >= string->length)
		end = string->length;
	return WLRTStringViewCreate(WLRTStringCData(string) + offset, end - offset);
}

bool WLRTStringResize(WLRTString* string, size_t newSize)
{
	if (!string ||
		!WLRTStringReserve(string, newSize))
		return false;

	if (newSize < string->length)
		return true;

	memset(WLRTStringData(string) + string->length, 0, newSize - string->length);
	string->length = newSize;
	return true;
}

bool WLRTStringReserve(WLRTString* string, size_t newCapacity)
{
	if (!string || newCapacity < string->length)
		return false;

	return WLRTStringEnsureCapacity(string, newCapacity);
}

bool WLRTStringAssign(WLRTString* string, WLRTStringView rhs)
{
	if (!string ||
		!WLRTStringEnsureCapacity(string, rhs.length))
		return false;

	char* data = WLRTStringData(string);
	memcpy(data, rhs.string, rhs.length);
	data[rhs.length] = '\0';
	string->length   = rhs.length;
	return true;
}

bool WLRTStringErase(WLRTString* string, size_t offset, size_t end)
{
	if (!string)
		return false;
	if (offset >= string->capacity)
		offset = string->capacity;
	if (end >= string->capacity)
		end = string->capacity;
	char* data = WLRTStringData(string);
	memmove(data + offset, data + end, string->capacity - end);
	memset(data + string->capacity - end, 0, string->capacity - end);
	string->length -= end - offset;
	return true;
}

bool WLRTStringInsert(WLRTString* string, size_t offset, WLRTStringView rhs)
{
	if (!string ||
		!WLRTStringEnsureCapacity(string, string->length + rhs.length))
		return false;
	if (offset >= string->capacity)
		offset = string->capacity;
	char* data = WLRTStringData(string);
	memcpy(data + offset, rhs.string, rhs.length);
	data[offset + rhs.length] = '\0';
	return true;
}

bool WLRTStringReplace(WLRTString* string, size_t offset, size_t end, WLRTStringView replacement)
{
	if (!string)
		return false;
	if (offset >= string->capacity)
		offset = string->capacity;
	if (end >= string->capacity)
		end = string->capacity;
	size_t requiredSize = string->length - (end - offset) + replacement.length;
	if (!WLRTStringEnsureCapacity(string, requiredSize))
		return false;
	char* data = WLRTStringData(string);
	memmove(data + offset + replacement.length, data + end, string->capacity - end);
	memcpy(data + offset, replacement.string, replacement.length);
	string->length       = requiredSize;
	data[string->length] = '\0';
	return true;
}

bool WLRTStringPrepend(WLRTString* string, WLRTStringView rhs)
{
	if (!string ||
		!WLRTStringEnsureCapacity(string, string->length + rhs.length))
		return false;
	char* data = WLRTStringData(string);
	memmove(data + rhs.length, data, string->length);
	memcpy(data, rhs.string, rhs.length);
	string->length      += rhs.length;
	data[string->length] = '\0';
	return true;
}

bool WLRTStringAppend(WLRTString* string, WLRTStringView rhs)
{
	if (!string ||
		!WLRTStringEnsureCapacity(string, string->length + rhs.length))
		return false;
	char* data = WLRTStringData(string);
	memcpy(data + string->length, rhs.string, rhs.length);
	string->length      += rhs.length;
	data[string->length] = '\0';
	return true;
}

int WLRTStringCompare(const WLRTString* lhs, const WLRTString* rhs)
{
	return WLRTStringViewCompare(WLRTStringSubstr(lhs, 0, ~0ULL), WLRTStringSubstr(rhs, 0, ~0ULL));
}

WLRTStringView WLRTStringViewCreate(const char* string, size_t length)
{
	if (length == ~0ULL)
		length = strlen(string);

	WLRTStringView view = {
		.string = string,
		.length = length
	};
	return view;
}

WLRTStringView WLRTStringViewSubstr(WLRTStringView string, size_t offset, size_t end)
{
	if (offset >= string.length)
		offset = string.length;
	if (end >= string.length)
		end = string.length;

	WLRTStringView view = {
		.string = string.string + offset,
		.length = end - offset
	};
	return view;
}

int WLRTStringViewCompare(WLRTStringView lhs, WLRTStringView rhs)
{
	if (!lhs.string && rhs.string)
		return -1;
	if (lhs.string && !rhs.string)
		return 1;
	if (!lhs.string && !rhs.string)
		return 0;
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

WLRTString WLRTStringFormat(WLRTStringView format, ...)
{
	va_list args;
	va_start(args, format);
	WLRTString res = WLRTStringFormatV(format, args);
	va_end(args);
	return res;
}

WLRTString WLRTStringFormatV(WLRTStringView format, va_list args)
{
	WLRTString formatCopy = WLRTStringCreate(format.string, format.length);
	va_list    copy;
	va_copy(copy, args);
	size_t len = vsnprintf(NULL, 0, WLRTStringCData(&formatCopy), copy);
	va_end(copy);
	WLRTString result;
	WLRTStringSetup(&result, len);
	result.length = vsnprintf(WLRTStringData(&result), result.capacity, WLRTStringCData(&formatCopy), args);
	return result;
}