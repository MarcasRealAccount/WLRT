#include "Window.h"
#include "Vk.h"

#include <stdio.h>
#include <stdlib.h>

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

static VkSurfaceFormatKHR VkSelectSurfaceFormat(VkData* vk, VkSwapchainData* swapchain)
{
	VkSurfaceFormatKHR selectedFormat = {
		.format     = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	};

	uint32_t formatCount = 0;
	if (!VkValidate(vk, vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physicalDevice, swapchain->surface, &formatCount, NULL))) return selectedFormat;
	VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*) malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	if (!formats)
	{
		VkReportError(vk, VK_ERROR_CODE_ALLOCATION_FAILURE, "Failed to allocate formats buffer");
		return selectedFormat;
	}
	if (!VkValidate(vk, vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physicalDevice, swapchain->surface, &formatCount, formats)))
	{
		free(formats);
		return selectedFormat;
	}
	for (uint32_t i = 0; i < formatCount; ++i)
	{
		VkSurfaceFormatKHR format = formats[i];
		if (format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			selectedFormat = format;
			break;
		}
	}
	free(formats);
	return selectedFormat;
}

static VkPresentModeKHR VkSelectPresentMode(VkData* vk, VkSwapchainData* swapchain)
{
	VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;

	uint32_t presentCount = 0;
	if (!VkValidate(vk, vkGetPhysicalDeviceSurfacePresentModesKHR(vk->physicalDevice, swapchain->surface, &presentCount, NULL))) return selectedMode;
	VkPresentModeKHR* presentModes = (VkPresentModeKHR*) malloc(presentCount * sizeof(VkPresentModeKHR));
	if (!presentModes)
	{
		VkReportError(vk, VK_ERROR_CODE_ALLOCATION_FAILURE, "Failed to allocate present modes");
		return selectedMode;
	}
	if (!VkValidate(vk, vkGetPhysicalDeviceSurfacePresentModesKHR(vk->physicalDevice, swapchain->surface, &presentCount, presentModes)))
	{
		free(presentModes);
		return selectedMode;
	}
	for (uint32_t i = 0; i < presentCount; ++i)
	{
		VkPresentModeKHR presentMode = presentModes[i];
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			selectedMode = presentMode;
			break;
		}
	}
	free(presentModes);
	return selectedMode;
}

bool VkSetupSwapchain(VkData* vk, VkSwapchainData* swapchain)
{
	if (!vk || !swapchain || !swapchain->window)
		return false;

	if (!swapchain->surface)
	{
		if (!VkValidate(vk, glfwCreateWindowSurface(vk->instance, swapchain->window->handle, vk->allocation, &swapchain->surface))) return false;
	}

	if (swapchain->swapchain)
	{
		for (uint32_t i = 0; i < swapchain->imageCount; ++i)
		{
			if (swapchain->views) vkDestroyImageView(vk->device, swapchain->views[i], vk->allocation);
		}
		free(swapchain->images);
		free(swapchain->views);
		swapchain->images = NULL;
		swapchain->views  = NULL;
	}

	VkSwapchainKHR oldSwapchain = swapchain->swapchain;

	VkSurfaceCapabilitiesKHR caps;
	if (!VkValidate(vk, vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->physicalDevice, swapchain->surface, &caps)))
	{
		VkCleanupSwapchain(vk, swapchain);
		return false;
	}
	uint32_t imageCount       = min(caps.minImageCount + 1, caps.maxImageCount);
	bool     imageCountDiffer = imageCount != swapchain->imageCount;
	if (imageCountDiffer)
	{
		vkDeviceWaitIdle(vk->device);
		for (uint32_t i = 0; i < swapchain->imageCount; ++i)
		{
			if (swapchain->imageAvailable) vkDestroySemaphore(vk->device, swapchain->imageAvailable[i], vk->allocation);
			if (swapchain->renderFinished) vkDestroySemaphore(vk->device, swapchain->renderFinished[i], vk->allocation);
		}
		free(swapchain->imageAvailable);
		free(swapchain->renderFinished);
		swapchain->imageAvailable = NULL;
		swapchain->renderFinished = NULL;
	}
	swapchain->imageCount  = imageCount;
	swapchain->format      = VkSelectSurfaceFormat(vk, swapchain);
	swapchain->presentMode = VkSelectPresentMode(vk, swapchain);
	swapchain->extent      = caps.currentExtent;

	VkSwapchainCreateInfoKHR createInfo = {
		.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext                 = NULL,
		.flags                 = 0,
		.surface               = swapchain->surface,
		.minImageCount         = swapchain->imageCount,
		.imageFormat           = swapchain->format.format,
		.imageColorSpace       = swapchain->format.colorSpace,
		.imageExtent           = swapchain->extent,
		.imageArrayLayers      = 1,
		.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL,
		.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode           = swapchain->presentMode,
		.clipped               = VK_FALSE,
		.oldSwapchain          = oldSwapchain
	};
	if (!VkValidate(vk, vkCreateSwapchainKHR(vk->device, &createInfo, vk->allocation, &swapchain->swapchain)))
	{
		VkCleanupSwapchain(vk, swapchain);
		return false;
	}
	if (oldSwapchain) vkDestroySwapchainKHR(vk->device, oldSwapchain, vk->allocation);
	if (!VkValidate(vk, vkGetSwapchainImagesKHR(vk->device, swapchain->swapchain, &swapchain->imageCount, NULL)))
	{
		VkCleanupSwapchain(vk, swapchain);
		return false;
	}
	swapchain->images = (VkImage*) malloc(swapchain->imageCount * sizeof(VkImage));
	swapchain->views  = (VkImageView*) malloc(swapchain->imageCount * sizeof(VkImageView));
	if (!swapchain->images || !swapchain->views)
	{
		VkCleanupSwapchain(vk, swapchain);
		VkReportError(vk, VK_ERROR_CODE_ALLOCATION_FAILURE, "Failed to allocate swapchain buffers");
		return false;
	}
	if (!VkValidate(vk, vkGetSwapchainImagesKHR(vk->device, swapchain->swapchain, &swapchain->imageCount, swapchain->images)))
	{
		VkCleanupSwapchain(vk, swapchain);
		return false;
	}
	for (uint32_t i = 0; i < swapchain->imageCount; ++i)
	{
		VkImageViewCreateInfo ivCreateInfo = {
			.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext            = NULL,
			.flags            = 0,
			.image            = swapchain->images[i],
			.viewType         = VK_IMAGE_VIEW_TYPE_2D,
			.format           = swapchain->format.format,
			.components       = { 0, 0, 0, 0 },
			.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
		};
		if (!VkValidate(vk, vkCreateImageView(vk->device, &ivCreateInfo, vk->allocation, swapchain->views + i)))
		{
			VkCleanupSwapchain(vk, swapchain);
			return false;
		}
	}
	if (imageCountDiffer)
	{
		swapchain->imageAvailable = (VkSemaphore*) malloc(swapchain->imageCount * sizeof(VkSemaphore));
		swapchain->renderFinished = (VkSemaphore*) malloc(swapchain->imageCount * sizeof(VkSemaphore));
		for (uint32_t i = 0; i < swapchain->imageCount; ++i)
		{
			VkSemaphoreCreateInfo sCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0
			};
			if (!VkValidate(vk, vkCreateSemaphore(vk->device, &sCreateInfo, vk->allocation, swapchain->imageAvailable + i)) ||
				!VkValidate(vk, vkCreateSemaphore(vk->device, &sCreateInfo, vk->allocation, swapchain->renderFinished + i)))
			{
				VkCleanupSwapchain(vk, swapchain);
				return false;
			}
		}
	}
	swapchain->invalid = false;
	return true;
}

void VkCleanupSwapchain(VkData* vk, VkSwapchainData* swapchain)
{
	for (uint32_t i = 0; i < swapchain->imageCount; ++i)
	{
		if (swapchain->views) vkDestroyImageView(vk->device, swapchain->views[i], vk->allocation);
		if (swapchain->imageAvailable) vkDestroySemaphore(vk->device, swapchain->imageAvailable[i], vk->allocation);
		if (swapchain->renderFinished) vkDestroySemaphore(vk->device, swapchain->renderFinished[i], vk->allocation);
	}
	vkDestroySwapchainKHR(vk->device, swapchain->swapchain, vk->allocation);
	vkDestroySurfaceKHR(vk->instance, swapchain->surface, vk->allocation);
}