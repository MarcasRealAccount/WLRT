#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct FSPath
{
	char*  buf;
	size_t len;
	size_t cap;
} FSPath;

FSPath FSCreatePath(const char* path, size_t length);
void   FSDestroyPath(FSPath* path);
bool   FSPathAppend(FSPath* lhs, const FSPath* rhs);
FSPath FSPathGetStem(const FSPath* path);
bool   FSPathEquals(const FSPath* lhs, const FSPath* rhs);

uint64_t FSLastWriteTime(const FSPath* filepath);
void     FSSetLastWriteTime(const FSPath* filepath, uint64_t time);
bool     FSCreateDirectories(const FSPath* directory);