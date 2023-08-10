#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void GLFWWinPosCB(GLFWwindow* window, int x, int y)
{
	WindowData* wd = (WindowData*) glfwGetWindowUserPointer(window);
	wd->x          = x;
	wd->y          = y;
}

void GLFWWinSizeCB(GLFWwindow* window, int width, int height)
{
	WindowData* wd = (WindowData*) glfwGetWindowUserPointer(window);
	wd->width      = width;
	wd->height     = height;
}

void GLFWWinCloseCB(GLFWwindow* window)
{
	WindowData* wd = (WindowData*) glfwGetWindowUserPointer(window);
	wd->wantsClose = true;
}

int WLRTCreateWindow(WindowData* wd)
{
	if (wd->handle != NULL)
		return 0;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	wd->handle = glfwCreateWindow(wd->width, wd->height, "WLRT", NULL, NULL);
	if (!wd->handle)
		return 0;
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

	return 1;
}

void WLRTDestroyWindow(WindowData* wd)
{
	if (wd->handle)
		glfwDestroyWindow(wd->handle);
}

int WLRTMakeWindowVisible(WindowData* wd)
{
	if (!wd->handle)
		return 0;

	glfwShowWindow(wd->handle);
	return 1;
}

int WLRTWindowPollEvents()
{
	glfwPollEvents();
	return 1;
}