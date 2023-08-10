#include "Vk.h"

#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

const char* VkGetErrorString(int code)
{
	switch (code)
	{
	case VK_ERROR_CODE_CALL_FAILURE: return "Call Failure";
	case VK_ERROR_CODE_ALLOCATION_FAILURE: return "Allocation Failure";
	case VK_ERROR_CODE_NO_PHYSICAL_DEVICES: return "No Physical Devices";
	default: return "Unknown";
	}
}

const char* VkGetResultString(VkResult result)
{
	switch (result)
	{
	case VK_SUCCESS: return "VK_SUCCESS";
	case VK_NOT_READY: return "VK_NOT_READY";
	case VK_TIMEOUT: return "VK_TIMEOUT";
	case VK_EVENT_SET: return "VK_EVENT_SET";
	case VK_EVENT_RESET: return "VK_EVENT_RESET";
	case VK_INCOMPLETE: return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
	case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
	case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
	case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
	case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
	case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
	case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT: return "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
	case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
	default: return "VK_ERROR_UNKNOWN";
	}
}

void VkReportError(VkData* vk, int code, const char* msg)
{
	if (vk->errorCallback)
		vk->errorCallback(code, msg);
}

bool VkValidate(VkData* vk, VkResult result)
{
	return VkValidateAllowed(vk, result, NULL, 0);
}

bool VkValidateAllowed(VkData* vk, VkResult result, VkResult* allowed, uint32_t allowedCount)
{
	if (!vk)
		return false;

	vk->lastResult = result;
	if (result >= VK_SUCCESS)
		return true;

	if (allowed && allowedCount > 0)
	{
		for (uint32_t i = 0; i < allowedCount; ++i)
			if (result == allowed[i])
				return true;
	}

	if (vk->errorCallback)
	{
		const char* resultStr = VkGetResultString(result);
		int         bufSize   = snprintf(NULL, 0, "Vulkan failed with (%08X) %s", (uint32_t) result, resultStr) + 1;

		char* buf = (char*) malloc(bufSize * sizeof(char));
		snprintf(buf, bufSize, "Vulkan failed with (%08X) %s", (uint32_t) result, resultStr);
		vk->errorCallback(VK_ERROR_CODE_CALL_FAILURE, buf);
		free(buf);
	}
	return false;
}

static bool VkSetupInstance(VkData* vk)
{
	uint32_t     extsCount = 0;
	const char** exts      = glfwGetRequiredInstanceExtensions(&extsCount);

	VkApplicationInfo appInfo = {
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext              = NULL,
		.pApplicationName   = "WLRT",
		.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
		.pEngineName        = "WLRT",
		.engineVersion      = VK_MAKE_API_VERSION(0, 0, 1, 0),
		.apiVersion         = VK_API_VERSION_1_3
	};
	VkInstanceCreateInfo createInfo = {
		.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext                   = NULL,
		.pApplicationInfo        = &appInfo,
		.enabledLayerCount       = 0,
		.ppEnabledLayerNames     = NULL,
		.enabledExtensionCount   = extsCount,
		.ppEnabledExtensionNames = exts
	};
	if (!VkValidate(vk, vkCreateInstance(&createInfo, vk->allocation, &vk->instance))) return false;
	return true;
}

static bool VkSelectPhysicalDevice(VkData* vk)
{
	uint32_t count = 0;
	if (!VkValidate(vk, vkEnumeratePhysicalDevices(vk->instance, &count, NULL))) return false;
	VkPhysicalDevice* devices = (VkPhysicalDevice*) malloc(count * sizeof(VkPhysicalDevice));
	if (!devices)
	{
		VkReportError(vk, VK_ERROR_CODE_ALLOCATION_FAILURE, "Failed to allocate Physical Devices");
		return false;
	}
	if (!VkValidate(vk, vkEnumeratePhysicalDevices(vk->instance, &count, devices)))
	{
		free(devices);
		return false;
	}

	for (uint32_t i = 0; i < count; ++i)
	{
		VkPhysicalDevice           device = devices[i];
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			vk->physicalDevice = device;
			break;
		}
	}
	if (!vk->physicalDevice)
		vk->physicalDevice = devices[0];
	free(devices);

	vk->deviceAccStructureProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	vk->deviceAccStructureProps.pNext = NULL;
	vk->deviceProps.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	vk->deviceProps.pNext             = &vk->deviceAccStructureProps;
	vkGetPhysicalDeviceProperties2(vk->physicalDevice, &vk->deviceProps);
	return true;
}

static bool VkSetupDevice(VkData* vk)
{
	const char* exts[] = {
		"VK_KHR_swapchain",
		"VK_KHR_deferred_host_operations",
		"VK_KHR_acceleration_structure"
	};

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accFeatures = {
		.sType                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
		.pNext                 = NULL,
		.accelerationStructure = VK_TRUE
	};
	VkPhysicalDeviceVulkan13Features features13 = {
		.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext            = &accFeatures,
		.synchronization2 = VK_TRUE,
		.dynamicRendering = VK_TRUE
	};
	VkPhysicalDeviceVulkan12Features features12 = {
		.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext             = &features13,
		.timelineSemaphore = VK_TRUE
	};
	VkPhysicalDeviceVulkan11Features features11 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.pNext = &features12
	};
	VkPhysicalDeviceFeatures2 features2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &features11
	};

	float                   prio            = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {
		.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext            = NULL,
		.flags            = 0,
		.queueFamilyIndex = 0,
		.queueCount       = 1,
		.pQueuePriorities = &prio
	};
	VkDeviceCreateInfo createInfo = {
		.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext                   = &features2,
		.flags                   = 0,
		.queueCreateInfoCount    = 1,
		.pQueueCreateInfos       = &queueCreateInfo,
		.enabledLayerCount       = 0,
		.ppEnabledLayerNames     = NULL,
		.enabledExtensionCount   = sizeof(exts) / sizeof(*exts),
		.ppEnabledExtensionNames = exts,
		.pEnabledFeatures        = NULL
	};
	if (!VkValidate(vk, vkCreateDevice(vk->physicalDevice, &createInfo, vk->allocation, &vk->device))) return false;
	vkGetDeviceQueue(vk->device, 0, 0, &vk->queue);
	return true;
}

static bool VkSetupVMA(VkData* vk)
{
	VmaAllocatorCreateInfo createInfo = {
		.flags                       = 0,
		.physicalDevice              = vk->physicalDevice,
		.device                      = vk->device,
		.preferredLargeHeapBlockSize = 0,
		.pAllocationCallbacks        = vk->allocation,
		.pDeviceMemoryCallbacks      = NULL,
		.pHeapSizeLimit              = NULL,
		.pVulkanFunctions            = NULL,
		.instance                    = vk->instance,
		.vulkanApiVersion            = VK_API_VERSION_1_3
	};
	if (!VkValidate(vk, vmaCreateAllocator(&createInfo, &vk->allocator))) return false;
	return true;
}

bool VkSetup(VkData* vk)
{
	if (!VkSetupInstance(vk) ||
		!VkSelectPhysicalDevice(vk) ||
		!VkSetupDevice(vk) ||
		!VkSetupVMA(vk) ||
		!VkSetupFrames(vk))
	{
		VkCleanup(vk);
		return false;
	}

	return true;
}

void VkCleanup(VkData* vk)
{
	VkCleanupFrames(vk);
	vmaDestroyAllocator(vk->allocator);
	vkDestroyDevice(vk->device, vk->allocation);
	vkDestroyInstance(vk->instance, vk->allocation);
	vk->allocator      = NULL;
	vk->queue          = NULL;
	vk->device         = NULL;
	vk->physicalDevice = NULL;
	vk->instance       = NULL;
}

bool VkSetupFrames(VkData* vk)
{
	if (vk->framesInFlight == vk->framesCapacity)
		return true;

	VkCleanupFrames(vk);

	vk->frames = (VkFrameData*) malloc(vk->framesInFlight * sizeof(VkFrameData));
	if (!vk->frames)
	{
		VkReportError(vk, VK_ERROR_CODE_ALLOCATION_FAILURE, "Failed to allocate frame data");
		return false;
	}
	vk->framesCapacity = vk->framesInFlight;
	for (uint32_t i = 0; i < vk->framesCapacity; ++i)
	{
		VkFrameData* frame = vk->frames + i;

		VkCommandPoolCreateInfo pCreateInfo = {
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext            = NULL,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = 0
		};
		if (!VkValidate(vk, vkCreateCommandPool(vk->device, &pCreateInfo, vk->allocation, &frame->pool)))
		{
			VkCleanupFrames(vk);
			return false;
		}
		VkCommandBufferAllocateInfo allocInfo = {
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext              = NULL,
			.commandPool        = frame->pool,
			.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
		if (!VkValidate(vk, vkAllocateCommandBuffers(vk->device, &allocInfo, &frame->buffer)))
		{
			VkCleanupFrames(vk);
			return false;
		}

		VkSemaphoreTypeCreateInfo stCreateInfo = {
			.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext         = NULL,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue  = 0
		};
		VkSemaphoreCreateInfo sCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &stCreateInfo,
			.flags = 0
		};
		if (!VkValidate(vk, vkCreateSemaphore(vk->device, &sCreateInfo, vk->allocation, &frame->semaphore)))
		{
			VkCleanupFrames(vk);
			return false;
		}
		frame->value = 0;
	}
	return true;
}

void VkCleanupFrames(VkData* vk)
{
	if (vk->frames)
	{
		do {
			VkSemaphore* semaphores = (VkSemaphore*) malloc(vk->framesCapacity * sizeof(VkSemaphore));
			uint64_t*    values     = (uint64_t*) malloc(vk->framesCapacity * sizeof(uint64_t));
			if (!semaphores || !values)
			{
				free(semaphores);
				free(values);
				break;
			}
			VkSemaphoreWaitInfo waitInfo = {
				.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
				.pNext          = NULL,
				.flags          = 0,
				.semaphoreCount = vk->framesCapacity,
				.pSemaphores    = semaphores,
				.pValues        = values
			};
			for (uint32_t i = 0; i < vk->framesCapacity; ++i)
			{
				VkFrameData* frame = vk->frames + i;
				semaphores[i]      = frame->semaphore;
				values[i]          = frame->value;
			}
			VkValidate(vk, vkWaitSemaphores(vk->device, &waitInfo, ~0ULL));
		}
		while (false);

		for (uint32_t i = 0; i < vk->framesCapacity; ++i)
		{
			VkFrameData* frame = vk->frames + i;
			vkDestroyCommandPool(vk->device, frame->pool, vk->allocation);
			vkDestroySemaphore(vk->device, frame->semaphore, vk->allocation);
		}
	}
	free(vk->frames);
	vk->frames         = NULL;
	vk->framesCapacity = 0;
}