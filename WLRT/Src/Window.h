#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct GLFWwindow GLFWwindow;

typedef struct WLRTWindowData
{
	GLFWwindow* handle;

	int32_t  x, y;
	uint32_t width, height;

	bool wantsClose;
} WLRTWindowData;

void WLRTWindowPollEvents();

bool WLRTWindowSetup(WLRTWindowData* window);
void WLRTWindowCleanup(WLRTWindowData* window);
void WLRTWindowSetVisible(WLRTWindowData* window, bool visible);