#pragma once

#include <stdbool.h>

typedef struct WLRTPath
{
	char*  path;
	size_t length;
	size_t capacity;
} WLRTPath;

bool     WLRTPathSetup(WLRTPath* path);
void     WLRTPathCleanup(WLRTPath* path);
WLRTPath WLRTPathCopy(const WLRTPath* path);
WLRTPath WLRTPathGetDirectory(const WLRTPath* path);
int      WLRTPathCompare(const WLRTPath* lhs, const WLRTPath* rhs);