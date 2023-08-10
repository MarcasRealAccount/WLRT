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

static void VKErrCB(int code, const char* msg)
{
	printf("VK ERROR (%d %s): %s\n", code, VkGetErrorString(code), msg);
}

int main(int argc, char** argv)
{
	glfwSetErrorCallback(&GLFWErrCB);
	if (!glfwInit())
		return 1;

	VkData vk;
	memset(&vk, 0, sizeof(vk));
	vk.framesInFlight = 2;
	vk.errorCallback  = &VKErrCB;
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

		VkSwapchainData* swapchains[] = { &vkSwapchain };
		if (!VkBeginFrame(&vk, swapchains, sizeof(swapchains) / sizeof(*swapchains))) break;

		VkFrameData* frame = VkGetCurrentFrame(&vk);
		for (uint32_t i = 0; i < sizeof(swapchains) / sizeof(*swapchains); ++i)
		{
			VkSwapchainData* swapchain = swapchains[i];

			VkRenderingAttachmentInfo colorAttachment = {
				.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.pNext              = NULL,
				.imageView          = swapchain->views[swapchain->imageIndex],
				.imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.resolveMode        = VK_RESOLVE_MODE_NONE,
				.resolveImageView   = NULL,
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue.color   = {186.0f / 255.0f, 218.0f / 255.0f, 85.0f / 255.0f, 1.0f}
			};
			VkRenderingInfo renderingInfo = {
				.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.pNext                = NULL,
				.flags                = 0,
				.renderArea.offset    = {0, 0},
				.renderArea.extent    = swapchains[i]->extent,
				.layerCount           = 1,
				.viewMask             = 0,
				.colorAttachmentCount = 1,
				.pColorAttachments    = &colorAttachment,
				.pDepthAttachment     = NULL,
				.pStencilAttachment   = NULL
			};
			vkCmdBeginRendering(frame->buffer, &renderingInfo);

			vkCmdEndRendering(frame->buffer);
		}

		if (!VkEndFrame(&vk)) break;
	}

	vkDeviceWaitIdle(vk.device);

	VkCleanupSwapchain(&vk, &vkSwapchain);
	WLRTDestroyWindow(&window);

	VkCleanup(&vk);

	glfwTerminate();
	return 0;
}