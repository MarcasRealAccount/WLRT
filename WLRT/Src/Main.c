#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>

void GLFWErrCB(int code, const char* msg)
{
	printf("GLFW ERROR (%d): %s\n", code, msg);
}

int main(int argc, char** argv)
{
	glfwSetErrorCallback(&GLFWErrCB);
	if (!glfwInit())
		return 1;

	WindowData window = {
		.handle = NULL,
		.x      = 1 << 31,
		.y      = 1 << 31,
		.width  = 1280,
		.height = 720
	};
	if (!WLRTCreateWindow(&window))
		return 1;

	if (!WLRTMakeWindowVisible(&window))
		return 1;

	while (!window.wantsClose)
	{
		if (!WLRTWindowPollEvents())
			break;
	}

	WLRTDestroyWindow(&window);

	glfwTerminate();
	return 0;
}