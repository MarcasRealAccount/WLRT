#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct GLFWwindow GLFWwindow;

typedef struct WindowData
{
	GLFWwindow* handle;

	int32_t  x, y;
	uint32_t width, height;

	bool wantsClose;
} WindowData;

bool WLRTCreateWindow(WindowData* wd);
void WLRTDestroyWindow(WindowData* wd);
void WLRTMakeWindowVisible(WindowData* wd);
void WLRTWindowPollEvents();