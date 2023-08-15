#include "Filesystem.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>

WLRTPath WLRTPathExec()
{
	wchar_t* wbuffer = (wchar_t*) malloc(32768 * sizeof(wchar_t));
	if (!wbuffer)
		return WLRTPathCreate(NULL, 0);
	DWORD len = GetModuleFileNameW(NULL, wbuffer, 32767);
	while (len > 0 && wbuffer[len - 1] != L'\\')
		--len;
	char* buffer = (char*) malloc(len * sizeof(wchar_t));
	if (!buffer)
	{
		free(wbuffer);
		return WLRTPathCreate(NULL, 0);
	}
	int      bufLen = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, buffer, len * sizeof(wchar_t), NULL, NULL);
	WLRTPath path   = WLRTPathCreate(buffer, bufLen);
	free(wbuffer);
	free(buffer);
	return path;
}

WLRTPath WLRTPathCWD()
{
	wchar_t* wbuffer = (wchar_t*) malloc(32768 * sizeof(wchar_t));
	if (!wbuffer)
		return WLRTPathCreate(NULL, 0);
	DWORD len    = GetCurrentDirectoryW(32767, wbuffer);
	char* buffer = (char*) malloc(len * sizeof(wchar_t));
	if (!buffer)
	{
		free(wbuffer);
		return WLRTPathCreate(NULL, 0);
	}
	int      bufLen = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, buffer, len * sizeof(wchar_t), NULL, NULL);
	WLRTPath path   = WLRTPathCreate(buffer, bufLen);
	free(wbuffer);
	free(buffer);
	return path;
}

void WLRTPathSetCWD(const WLRTPath* path)
{
	if (!path || !path->path)
		return;
	SetCurrentDirectoryA(path->path);
}

bool WLRTPathSetup(WLRTPath* path)
{
	path->path     = NULL;
	path->length   = 0;
	path->capacity = 0;
	return true;
}

void WLRTPathCleanup(WLRTPath* path)
{
	free(path->path);
	path->path     = NULL;
	path->length   = 0;
	path->capacity = 0;
}

static bool WLRTPathEnsureSize(WLRTPath* path, size_t size)
{
	if (size <= path->capacity)
		return true;

	size_t newCapacity = size;
	newCapacity       |= newCapacity >> 1;
	newCapacity       |= newCapacity >> 2;
	newCapacity       |= newCapacity >> 4;
	newCapacity       |= newCapacity >> 8;
	newCapacity       |= newCapacity >> 16;
	newCapacity       |= newCapacity >> 32;
	++newCapacity;
	char* newBuf = (char*) malloc(newCapacity);
	if (!newBuf)
		return false;

	memcpy(newBuf, path->path, path->length);
	newBuf[path->length] = '\0';
	free(path->path);
	path->capacity = newCapacity;
	path->path     = newBuf;
	return true;
}

static void WLRTPathResolve(WLRTPath* path)
{
	HANDLE handle         = NULL;
	size_t previousOffset = 0;
	size_t offset         = 0;
	while (offset < path->length)
	{
		size_t end = offset;
		while (end < path->length && path->path[end] != '/')
			++end;
		if (end >= path->length)
			break;
		size_t len = end - offset;
		switch (len)
		{
		case 0: // Can only be /
			WLRTPathErase(path, offset, end + 1);
			break;
		case 1: // Could be ./
			if (path->path[offset] == '.')
			{
				WLRTPathErase(path, offset, end + 1);
			}
			else
			{
				previousOffset = offset;
				offset         = end + 1;
			}
			break;
		case 2: // Could be ../
			if (path->path[offset] == '.' && path->path[offset + 1] == '.')
			{
				WLRTPathErase(path, previousOffset, end + 1);
			}
			else
			{
				previousOffset = offset;
				offset         = end + 1;
			}
			break;
		default: // Normal directory
			previousOffset = offset;
			offset         = end + 1;
			break;
		}
	}
	switch (path->length - offset)
	{
	case 0: // Can only be /
		WLRTPathErase(path, offset, path->length + 1);
		break;
	case 1: // Can be ./
		if (path->path[offset] == '.')
			WLRTPathErase(path, offset, path->length + 1);
		break;
	case 2: // Can be ../
		if (path->path[offset] == '.' || path->path[offset + 1] == '.')
			WLRTPathErase(path, previousOffset, path->length + 1);
		break;
	}

	offset = path->length;
	while (offset > 0)
	{
		size_t end = offset;
		while (end > 0 && path->path[end - 1] != '/')
			--end;
		if (end == 0)
			break;
		path->path[end - 1] = '\0';
		handle              = CreateFileA(path->path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		path->path[end - 1] = '/';
		offset              = end - 1;
		if (handle && handle != INVALID_HANDLE_VALUE)
			break;
	}
	if (handle && handle != INVALID_HANDLE_VALUE)
	{
		do {
			wchar_t* wbuffer = (wchar_t*) malloc(32768 * sizeof(wchar_t));
			if (!wbuffer)
				break;
			DWORD len    = GetFinalPathNameByHandleW(handle, wbuffer, 32768, 0);
			char* buffer = (char*) malloc(len * sizeof(wchar_t) * sizeof(char));
			if (!buffer)
			{
				free(wbuffer);
				break;
			}
			int bufLen        = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, buffer, len * sizeof(wchar_t), NULL, NULL);
			buffer[bufLen]    = '\0';
			WLRTPath tempPath = {
				.path     = buffer,
				.length   = bufLen,
				.capacity = len * sizeof(wchar_t)
			};
			WLRTPathNormalize(&tempPath);
			WLRTPathReplace(path, 0, offset, &tempPath, ~0ULL);
			free(wbuffer);
			free(buffer);
		}
		while (false);
	}

	if (path->length >= 4 && path->path[0] == '/' && path->path[1] == '/' && path->path[2] == '?' && path->path[3] == '/')
		WLRTPathErase(path, 0, 4);
}

bool WLRTPathAbsolute(WLRTPath* path)
{
	if (!path)
		return false;

	if (path->length == 0)
	{
		WLRTPath temp = WLRTPathCWD();
		WLRTPathAssign(path, &temp);
		WLRTPathCleanup(&temp);
		return true;
	}

	if (path->length < 2)
	{
		WLRTPath temp = WLRTPathCWD();
		WLRTPathPrepend(path, &temp);
		WLRTPathCleanup(&temp);
		return true;
	}

	if ((path->path[0] == '/' && path->path[1] == '/') ||
		(((path->path[0] >= 'A' && path->path[0] <= 'Z') || (path->path[0] >= 'a' && path->path[0] <= 'z')) && path->path[1] == ':'))
	{
		WLRTPathResolve(path);
	}
	else
	{
		WLRTPath temp = WLRTPathCWD();
		WLRTPathPrepend(path, &temp);
		WLRTPathCleanup(&temp);
		WLRTPathResolve(path);
	}
	return true;
}

bool WLRTPathNormalize(WLRTPath* path)
{
	if (!path)
		return false;
	if (!path->path || path->length == 0)
		return true;

	for (size_t i = 0; i < path->length; ++i)
		if (path->path[i] == '\\')
			path->path[i] = '/';
	return true;
}

WLRTPath WLRTPathCreate(const char* path, size_t length)
{
	if (!path || length == 0)
	{
		WLRTPath result = {
			.path     = NULL,
			.length   = 0,
			.capacity = 0
		};
		return result;
	}

	if (length == ~0ULL)
		length = strlen(path);

	WLRTPath result = {
		.path     = NULL,
		.length   = 0,
		.capacity = 0
	};
	if (!WLRTPathEnsureSize(&result, length))
		return result;
	memcpy(result.path, path, length);
	result.length = length;
	WLRTPathNormalize(&result);
	result.path[result.length] = '\0';
	return result;
}

WLRTPath WLRTPathCopy(const WLRTPath* path)
{
	return path ? WLRTPathCreate(path->path, path->length) : WLRTPathCreate(NULL, 0);
}

bool WLRTPathAssign(WLRTPath* lhs, const WLRTPath* rhs)
{
	if (!lhs || !rhs)
		return false;

	if (!WLRTPathEnsureSize(lhs, rhs->length))
		return false;

	memcpy(lhs->path, rhs->path, rhs->length);
	lhs->length            = rhs->length;
	lhs->path[lhs->length] = '\0';
	return true;
}

bool WLRTPathErase(WLRTPath* path, size_t offset, size_t end)
{
	if (!path)
		return false;

	if (offset > path->length)
		offset = path->length;
	if (end > path->length)
		end = path->length;
	size_t toErase = end - offset;
	memmove(path->path + offset, path->path + end, path->length - end);
	memset(path->path + path->length - toErase, 0, toErase);
	path->length -= toErase;
	return true;
}

bool WLRTPathInsert(WLRTPath* path, size_t offset, const WLRTPath* rhs)
{
	if (!path || !rhs)
		return false;

	if (!WLRTPathEnsureSize(path, path->length + rhs->length))
		return false;

	memmove(path->path + offset + rhs->length, path->path + offset, path->length - offset);
	memcpy(path->path + offset, rhs->path, rhs->length);
	path->length            += rhs->length;
	path->path[path->length] = '\0';
	return true;
}

bool WLRTPathReplace(WLRTPath* path, size_t offset, size_t end, const WLRTPath* replacement, size_t length)
{
	if (!path || !replacement || !replacement->path)
		return false;

	if (offset > path->length)
		offset = path->length;
	if (end > path->length)
		end = path->length;
	if (length > replacement->length)
		length = replacement->length;
	size_t end2         = offset + length;
	size_t diff         = end - end2;
	size_t requiredSize = path->length - diff;
	if (!WLRTPathEnsureSize(path, requiredSize))
		return false;

	memmove(path->path + end2, path->path + end, path->length - end);
	if (requiredSize < path->length)
		memset(path->path + requiredSize, 0, path->length - requiredSize);
	else
		path->path[requiredSize] = '\0';
	memcpy(path->path + offset, replacement->path, length);
	path->length = requiredSize;
	return true;
}

bool WLRTPathPrepend(WLRTPath* lhs, const WLRTPath* rhs)
{
	if (!lhs || !rhs)
		return false;

	size_t offset1 = 0;
	size_t len1    = rhs->length;
	if (lhs->length > 0 && lhs->path[0] == '/')
		++offset1;
	if (rhs->length > 0 && rhs->path[len1 - 1] == '/')
		--len1;
	size_t requiredSize = lhs->length - offset1 + len1 + 1;
	if (!WLRTPathEnsureSize(lhs, requiredSize))
		return false;

	memmove(lhs->path + len1 + 1, lhs->path + offset1, lhs->length - offset1);
	lhs->path[len1] = '/';
	memcpy(lhs->path, rhs->path, len1);
	lhs->length            = requiredSize;
	lhs->path[lhs->length] = '\0';
	return true;
}

bool WLRTPathAppend(WLRTPath* lhs, const WLRTPath* rhs)
{
	if (!lhs || !rhs)
		return false;

	size_t len1    = lhs->length;
	size_t offset1 = 0;
	if (lhs->length > 0 && lhs->path[len1 - 1] == '/')
		--len1;
	if (rhs->length > 0 && lhs->path[0] == '/')
		++offset1;
	size_t requiredSize = len1 + rhs->length - offset1 + 1;
	if (!WLRTPathEnsureSize(lhs, requiredSize))
		return false;

	lhs->path[len1] = '/';
	memcpy(lhs->path + len1 + 1, rhs->path + offset1, rhs->length - offset1);
	lhs->length            = requiredSize;
	lhs->path[lhs->length] = '\0';
	return true;
}

WLRTPath WLRTPathGetDirectory(const WLRTPath* path)
{
	if (!path)
		return WLRTPathCreate(NULL, 0);
	size_t end = path->length;
	while (end > 0 && path->path[end - 1] != '/')
		--end;
	if (path->path[end] == '/')
		++end;
	if (end == 0)
		return WLRTPathCWD();
	return WLRTPathCreate(path->path, end);
}

int WLRTPathCompare(const WLRTPath* lhs, const WLRTPath* rhs)
{
	if (!lhs && rhs)
		return -1;
	else if (lhs && !rhs)
		return 1;
	else if (!lhs && !rhs)
		return 0;

	if (lhs->length < rhs->length)
		return -1;
	else if (lhs->length > rhs->length)
		return 1;

	for (size_t i = 0; i < lhs->length; ++i)
	{
		char a = lhs->path[i];
		char b = rhs->path[i];
		if (a < b)
			return -1;
		else if (a > b)
			return 1;
	}
	return 0;
}