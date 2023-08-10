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

int  WLRTCreateWindow(WindowData* wd);
void WLRTDestroyWindow(WindowData* wd);
int  WLRTMakeWindowVisible(WindowData* wd);
int  WLRTWindowPollEvents();