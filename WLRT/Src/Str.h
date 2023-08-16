#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct WLRTString
{
	size_t length;
	size_t capacity;
	char   sso[sizeof(size_t) * 2];
} WLRTString;

typedef struct WLRTStringView
{
	const char* string;
	size_t      length;
} WLRTStringView;

bool           WLRTStringSetup(WLRTString* string, size_t initialCapacity);
void           WLRTStringCleanup(WLRTString* string);
WLRTString     WLRTStringCreate(const char* string, size_t length);
WLRTString     WLRTStringCopy(WLRTStringView string);
char*          WLRTStringData(WLRTString* string);
const char*    WLRTStringCData(const WLRTString* string);
WLRTStringView WLRTStringSubstr(const WLRTString* string, size_t offset, size_t end);
bool           WLRTStringResize(WLRTString* string, size_t newSize);
bool           WLRTStringReserve(WLRTString* string, size_t newCapacity);
bool           WLRTStringAssign(WLRTString* string, WLRTStringView rhs);
bool           WLRTStringErase(WLRTString* string, size_t offset, size_t end);
bool           WLRTStringInsert(WLRTString* string, size_t offset, WLRTStringView rhs);
bool           WLRTStringReplace(WLRTString* string, size_t offset, size_t end, WLRTStringView replacement);
bool           WLRTStringPrepend(WLRTString* string, WLRTStringView rhs);
bool           WLRTStringAppend(WLRTString* string, WLRTStringView rhs);
int            WLRTStringCompare(const WLRTString* lhs, const WLRTString* rhs);

WLRTStringView WLRTStringViewCreate(const char* string, size_t length);
WLRTStringView WLRTStringViewSubstr(WLRTStringView string, size_t offset, size_t end);
int            WLRTStringViewCompare(WLRTStringView lhs, WLRTStringView rhs);

WLRTString WLRTStringFormat(WLRTStringView format, ...);
WLRTString WLRTStringFormatV(WLRTStringView format, va_list args);