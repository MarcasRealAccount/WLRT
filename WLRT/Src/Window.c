#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

static void GLFWWinPosCB(GLFWwindow* window, int x, int y)
{
	WindowData* wd = (WindowData*) glfwGetWindowUserPointer(window);
	wd->x          = x;
	wd->y          = y;
}

static void GLFWWinSizeCB(GLFWwindow* window, int width, int height)
{
	WindowData* wd = (WindowData*) glfwGetWindowUserPointer(window);
	wd->width      = width;
	wd->height     = height;
}

static void GLFWWinCloseCB(GLFWwindow* window)
{
	WindowData* wd = (WindowData*) glfwGetWindowUserPointer(window);
	wd->wantsClose = true;
}

bool WLRTCreateWindow(WindowData* wd)
{
	if (wd->handle != NULL)
		return false;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	wd->handle = glfwCreateWindow(wd->width, wd->height, "WLRT", NULL, NULL);
	if (!wd->handle)
		return false;
	glfwSetWindowUserPointer(wd->handle, wd);
	glfwSetWindowPosCallback(wd->handle, &GLFWWinPosCB);
	glfwSetWindowSizeCallback(wd->handle, &GLFWWinSizeCB);
	glfwSetWindowCloseCallback(wd->handle, &GLFWWinCloseCB);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	int32_t      mx, my, mw, mh;
	glfwGetMonitorWorkarea(monitor, &mx, &my, &mw, &mh);
	int32_t wx = wd->x;
	int32_t wy = wd->y;
	if (wx == 1 << 31) wx = mx + ((mw - wd->width) >> 1);
	if (wy == 1 << 31) wy = my + ((mh - wd->height) >> 1);
	glfwSetWindowPos(wd->handle, wx, wy);
	glfwMaximizeWindow(wd->handle);

	return true;
}

void WLRTDestroyWindow(WindowData* wd)
{
	if (wd->handle)
		glfwDestroyWindow(wd->handle);
}

void WLRTMakeWindowVisible(WindowData* wd)
{
	if (!wd->handle)
		return;

	glfwShowWindow(wd->handle);
}

void WLRTWindowPollEvents()
{
	glfwPollEvents();
}