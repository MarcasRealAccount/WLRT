#pragma once

#include <stdbool.h>

typedef void (*WLRTExitCallbackFn)(void* userdata);

bool WLRTExitSetup();
void WLRTExitHandle();
void WLRTExitRegister(WLRTExitCallbackFn callback, void* userdata);
void WLRTExitAssert(bool statement, int code);