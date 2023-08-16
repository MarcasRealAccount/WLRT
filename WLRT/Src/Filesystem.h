#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct WLRTPath
{
	char*  path;
	size_t length;
	size_t capacity;
} WLRTPath;

WLRTPath WLRTPathExec();
WLRTPath WLRTPathCWD();
void     WLRTPathSetCWD(const WLRTPath* path);

bool     WLRTPathSetup(WLRTPath* path);
void     WLRTPathCleanup(WLRTPath* path);
WLRTPath WLRTPathCreate(const char* path, size_t length);
WLRTPath WLRTPathCopy(const WLRTPath* path);
bool     WLRTPathAbsolute(WLRTPath* path);
bool     WLRTPathNormalize(WLRTPath* path);
bool     WLRTPathAssign(WLRTPath* path, const WLRTPath* rhs);
bool     WLRTPathErase(WLRTPath* path, size_t offset, size_t end);
bool     WLRTPathInsert(WLRTPath* path, size_t offset, const WLRTPath* rhs);
bool     WLRTPathReplace(WLRTPath* path, size_t offset, size_t end, const WLRTPath* replacement, size_t length);
bool     WLRTPathPrepend(WLRTPath* path, const WLRTPath* rhs);
bool     WLRTPathAppend(WLRTPath* path, const WLRTPath* rhs);
WLRTPath WLRTPathGetDirectory(const WLRTPath* path);
int      WLRTPathCompare(const WLRTPath* lhs, const WLRTPath* rhs);