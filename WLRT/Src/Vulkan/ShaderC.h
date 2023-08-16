#pragma once

#include "DynArray.h"
#include "FileSystem.h"
#include "Str.h"

#include <stdbool.h>

WLRTDynArray WLRTShaderCompile(const WLRTPath* filepath, WLRTStringView source);