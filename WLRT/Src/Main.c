#include "Vk.h"
#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>

static void GLFWErrCB(int code, const char* msg)
{
	printf("GLFW ERROR (%d): %s\n", code, msg);
}

int main(int argc, char** argv)
{
	glfwSetErrorCallback(&GLFWErrCB);
	if (!glfwInit())
		return 1;

	VkData vk;
	memset(&vk, 0, sizeof(vk));
	if (!VkSetup(&vk))
	{
		glfwTerminate();
		return 1;
	}

	WindowData window = {
		.handle = NULL,
		.x      = 1 << 31,
		.y      = 1 << 31,
		.width  = 1280,
		.height = 720
	};
	VkSwapchainData vkSwapchain;
	memset(&vkSwapchain, 0, sizeof(vkSwapchain));
	vkSwapchain.window = &window;
	if (!WLRTCreateWindow(&window))
	{
		VkCleanup(&vk);
		glfwTerminate();
		return 1;
	}
	if (!VkSetupSwapchain(&vk, &vkSwapchain))
	{
		WLRTDestroyWindow(&window);
		VkCleanup(&vk);
		glfwTerminate();
		return 1;
	}

	WLRTMakeWindowVisible(&window);

	while (!window.wantsClose)
	{
		WLRTWindowPollEvents();
	}

	VkCleanupSwapchain(&vk, &vkSwapchain);
	WLRTDestroyWindow(&window);

	VkCleanup(&vk);

	glfwTerminate();
	return 0;
}