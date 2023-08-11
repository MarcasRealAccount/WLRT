#pragma once

#include <stdbool.h>

typedef void (*ExitCallbackFn)(void* data);

bool ExitSetup();
void ExitHandle();
void ExitRegister(ExitCallbackFn callback, void* data);
void ExitAssert(bool statement, int code);