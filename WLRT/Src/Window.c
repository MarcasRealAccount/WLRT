#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

static void GLFWWindowPosCallback(GLFWwindow* window, int x, int y)
{
	WLRTWindowData* wd = (WLRTWindowData*) glfwGetWindowUserPointer(window);
	wd->x              = x;
	wd->y              = y;
}

static void GLFWWindowSizeCallback(GLFWwindow* window, int width, int height)
{
	WLRTWindowData* wd = (WLRTWindowData*) glfwGetWindowUserPointer(window);
	wd->width          = width;
	wd->height         = height;
}

static void GLFWWindowCloseCallback(GLFWwindow* window)
{
	WLRTWindowData* wd = (WLRTWindowData*) glfwGetWindowUserPointer(window);
	wd->wantsClose     = true;
}

void WLRTWindowPollEvents()
{
	glfwPollEvents();
}

bool WLRTWindowSetup(WLRTWindowData* window)
{
	if (!window)
		return false;

	window->handle     = NULL;
	window->wantsClose = false;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window->handle = glfwCreateWindow(window->width, window->height, "WLRT", NULL, NULL);
	if (!window->handle)
		return false;

	glfwSetWindowUserPointer(window->handle, window);
	glfwSetWindowPosCallback(window->handle, &GLFWWindowPosCallback);
	glfwSetWindowSizeCallback(window->handle, &GLFWWindowSizeCallback);
	glfwSetWindowCloseCallback(window->handle, &GLFWWindowCloseCallback);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	int32_t      mx, my, mw, mh;
	glfwGetMonitorWorkarea(monitor, &mx, &my, &mw, &mh);
	int32_t wx = window->x;
	int32_t wy = window->y;
	if (wx == 1 << 31) wx = mx + ((mw - window->width) >> 1);
	if (wy == 1 << 31) wy = my + ((mh - window->height) >> 1);
	glfwSetWindowPos(window->handle, wx, wy);
	glfwMaximizeWindow(window->handle);

	return true;
}

void WLRTWindowCleanup(WLRTWindowData* window)
{
	if (!window || !window->handle)
		return;

	glfwDestroyWindow(window->handle);
}

void WLRTWindowSetVisible(WLRTWindowData* window, bool visible)
{
	if (!window || !window->handle)
		return;

	if (visible)
		glfwShowWindow(window->handle);
	else
		glfwHideWindow(window->handle);
}