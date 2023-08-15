#include "Exit.h"
#include "FileWatcher.h"
#include "Window.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

static void GLFWErrorCallback(int code, const char* message)
{
	printf("GLFW Error (%d): %s\n", code, message);
}

static void GLFWOnExit(void* userdata)
{
	(void) userdata;
	glfwTerminate();
}

static void WindowOnExit(void* userdata)
{
	WLRTWindowCleanup((WLRTWindowData*) userdata);
}

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	if (!WLRTExitSetup())
		return 1;

	glfwSetErrorCallback(&GLFWErrorCallback);
	WLRTExitAssert(glfwInit(), 1);
	WLRTExitRegister(&GLFWOnExit, NULL);

	WLRTWindowData window = {
		.x      = 1 << 31,
		.y      = 1 << 31,
		.width  = 1280,
		.height = 720,
	};
	WLRTExitAssert(WLRTWindowSetup(&window), 1);
	WLRTExitRegister(&WindowOnExit, &window);
	WLRTWindowSetVisible(&window, true);

	while (!window.wantsClose)
	{
		WLRTWindowPollEvents();
	}

	WLRTExitHandle();
	return 0;
}