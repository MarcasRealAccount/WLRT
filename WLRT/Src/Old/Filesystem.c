#include "Filesystem.h"

#include <stdlib.h>

#include <Windows.h>

FSPath FSCreatePath(const char* path, size_t length)
{
	if (!path)
	{
		FSPath out = {
			.buf = NULL,
			.len = 0,
			.cap = 0
		};
		return out;
	}
	if (length == ~0ULL)
		length = strlen(path);

	FSPath out = {
		.buf = (char*) malloc((length + 1) * sizeof(char)),
		.len = length,
		.cap = length
	};
	if (!out.buf)
	{
		out.buf = NULL;
		out.len = 0;
		out.cap = 0;
		return out;
	}
	memcpy(out.buf, path, length);
	out.buf[out.len] = '\0';
	return out;
}

void FSDestroyPath(FSPath* path)
{
	if (!path)
		return;
	free(path->buf);
	path->buf = NULL;
	path->len = 0;
	path->cap = 0;
}

static bool FSPathEnsureSize(FSPath* path, size_t requiredSize)
{
	if (requiredSize <= path->cap)
		return true;

	char* newBuf = (char*) malloc((requiredSize + 1) * sizeof(char));
	if (!newBuf)
		return false;

	memcpy(newBuf, path->buf, path->len);
	free(path->buf);
	path->buf = newBuf;
	path->cap = requiredSize;
	return true;
}

bool FSPathAppend(FSPath* lhs, const FSPath* rhs)
{
	if (!lhs || !rhs)
		return false;

	size_t requiredSize = lhs->len + rhs->len + 1;
	FSPathEnsureSize(lhs, requiredSize);
	lhs->buf[lhs->len] = '/';
	memcpy(lhs->buf + lhs->len + 1, rhs->buf, rhs->len);
	lhs->len               = requiredSize;
	lhs->buf[requiredSize] = '\0';
	return true;
}

FSPath FSPathGetStem(const FSPath* path)
{
	if (!path)
	{
		FSPath out = {
			.buf = NULL,
			.len = 0,
			.cap = 0
		};
		return out;
	}

	size_t end = path->len;
	while (end > 0 && path->buf[end] != '/')
		--end;
	++end;
	FSPath out = {
		.buf = (char*) malloc((end + 1) * sizeof(char)),
		.len = end,
		.cap = end
	};
	if (!out.buf)
	{
		out.buf = NULL;
		out.len = 0;
		out.cap = 0;
		return out;
	}
	memcpy(out.buf, path->buf, end);
	out.buf[end] = '\0';
	return out;
}

bool FSPathEquals(const FSPath* lhs, const FSPath* rhs)
{
	if (!lhs || !rhs || lhs->len != rhs->len)
		return false;

	for (size_t i = 0; i < lhs->len; ++i)
		if (lhs->buf[i] != rhs->buf[i])
			return false;
	return true;
}

uint64_t FSLastWriteTime(const FSPath* filepath)
{
	if (!filepath)
		return 0;

	FILETIME createTime, accessTime, writeTime;
	HANDLE   fHandle = CreateFileA(filepath->buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (!GetFileTime(fHandle, &createTime, &accessTime, &writeTime))
	{
		CloseHandle(fHandle);
		return 0;
	}
	CloseHandle(fHandle);
	return writeTime.dwLowDateTime | ((uint64_t) writeTime.dwHighDateTime) << 32;
}

void FSSetLastWriteTime(const FSPath* filepath, uint64_t time)
{
	if (!filepath)
		return;

	FILETIME writeTime = {
		.dwLowDateTime  = time & 0xFFFF'FFFF,
		.dwHighDateTime = (time >> 32) & 0xFFFF'FFFF
	};
	HANDLE fHandle = CreateFileA(filepath->buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	SetFileTime(fHandle, NULL, NULL, &writeTime);
	CloseHandle(fHandle);
}

bool FSCreateDirectories(const FSPath* directories)
{
	size_t offset   = 0;
	FSPath stemPath = FSCreatePath(directories->buf, directories->len);
	if (!stemPath.buf)
		return false;
	bool replaceBack = false;
	while (true)
	{
		if (replaceBack)
			stemPath.buf[offset] = '/';
		++offset;
		while (offset < stemPath.len && stemPath.buf[offset] != '/')
			++offset;
		if (offset >= stemPath.len)
			break;
		stemPath.buf[offset] = '\0';
		replaceBack          = true;

		if (!CreateDirectoryA(stemPath.buf, NULL))
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
				FSDestroyPath(&stemPath);
				return false;
			}
		}
	}
	FSDestroyPath(&stemPath);
	return true;
}