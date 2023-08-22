#include "Memory.h"
#include "Vulkan.h"

const char* WLRTVulkanGetResultString(VkResult result)
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

bool WLRTVulkanValidate(WLRTVulkanData* vk, VkResult result)
{
	if (!vk)
		return false;
	vk->lastResult = result;
	if (result >= VK_SUCCESS)
		return true;

	WLRTLoggerErrorF(&vk->logger, WLRTStringViewCreate("(%d) %s", 7), (uint32_t) result, WLRTVulkanGetResultString(result));
	return false;
}

bool WLRTVulkanValidateAllowed(WLRTVulkanData* vk, VkResult result, VkResult* allowed, uint32_t allowedCount)
{
	if (!vk)
		return false;
	vk->lastResult = result;
	if (result >= VK_SUCCESS)
		return true;

	if (allowed)
	{
		for (uint32_t i = 0; i < allowedCount; ++i)
		{
			if (result == allowed[i])
				return true;
		}
	}

	WLRTLoggerErrorF(&vk->logger, WLRTStringViewCreate("(%d) %s", 7), (uint32_t) result, WLRTVulkanGetResultString(result));
	return false;
}

WLRTVulkanData* WLRTVulkanGetFrame(WLRTVulkanData* vk, uint32_t frame)
{
	return vk ? (WLRTVulkanData*) WLRTDynArrayGet(&vk->frames, frame) : NULL;
}

WLRTVulkanData* WLRTVulkanGetCurrentFrame(WLRTVulkanData* vk)
{
	return vk ? (WLRTVulkanData*) WLRTDynArrayGet(&vk->frames, vk->currentFrame) : NULL;
}

bool WLRTVulkanBeginFrame(WLRTVulkanData* vk, WLRTVulkanSwapchainData** swapchains, uint32_t swapchainCount)
{
	if (!vk)
		return false;
}

void WLRTVulkanEndFrame(WLRTVulkanData* vk)
{
	if (!vk)
		return;
}

static bool WLRTVulkanSetupInstance(WLRTVulkanData* vk)
{
	VkApplicationInfo    appInfo    = {};
	VkInstanceCreateInfo createInfo = {};
	return WLRTVulkanValidate(vk, vkCreateInstance(&createInfo, &vk->allocation, &vk->instance));
}

static bool WLRTVulkanSetupPhysicalDevice(WLRTVulkanData* vk)
{
	uint32_t count = 0;
	if (!WLRTVulkanValidate(vk, vkEnumeratePhysicalDevices(vk->instance, &count, NULL)))
		return false;
	if (!count)
		return false;
	VkPhysicalDevice* devices = WLRTAlloc(count * sizeof(VkPhysicalDevice), alignof(VkPhysicalDevice));
	if (!devices ||
		!WLRTVulkanValidate(vk, vkEnumeratePhysicalDevices(vk->instance, &count, devices)))
	{
		WLRTFree(devices, alignof(VkPhysicalDevice));
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

	vk->deviceProps.rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	vk->deviceProps.rayTracingPipelineProperties.pNext = NULL;

	vk->deviceProps.accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	vk->deviceProps.accelerationStructureProperties.pNext = &vk->deviceProps.rayTracingPipelineProperties;

	vk->deviceProps.properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	vk->deviceProps.properties2.pNext = &vk->deviceProps.accelerationStructureProperties;
	vkGetPhysicalDeviceProperties2(vk->physicalDevice, &vk->deviceProps.properties2);
	return true;
}

static bool WLRTVulkanSetupDevice(WLRTVulkanData* vk)
{
	VkDeviceQueueCreateInfo queueInfo  = {};
	VkDeviceCreateInfo      createInfo = {};
	if (!WLRTVulkanValidate(vk, vkCreateDevice(vk->physicalDevice, &createInfo, &vk->allocation, &vk->device)))
		return false;
	vkGetDeviceQueue(vk->device, 0, 0, &vk->queue);
}

static bool WLRTVulkanSetupAllocator(WLRTVulkanData* vk)
{
	VmaAllocatorCreateInfo createInfo = {};
	return WLRTVulkanValidate(vk, vmaCreateAllocator(&createInfo, &vk->allocator));
}

static bool WLRTVulkanSetupPipelineCache(WLRTVulkanData* vk)
{
	VkPipelineCacheCreateInfo createInfo = {};
	return WLRTVulkanValidate(vk, vkCreatePipelineCache(vk->device, &createInfo, &vk->allocation, &vk->pipelineCache));
}

static bool WLRTVulkanSetupFrames(WLRTVulkanData* vk)
{
}

static void WLRTVulkanCleanupPipelineCache(WLRTVulkanData* vk)
{
	vkDestroyPipelineCache(vk->device, vk->pipelineCache, &vk->allocation);
}

static void WLRTVulkanCleanupFrames(WLRTVulkanData* vk)
{
}

bool WLRTVulkanSetup(WLRTVulkanData* vk)
{
	if (!vk ||
		!WLRTLoggerSetup(&vk->logger, WLRTStringViewCreate("Vulkan", 6)))
		return false;
	if (!WLRTVulkanSetupInstance(vk) ||
		!WLRTVulkanSetupPhysicalDevice(vk) ||
		!WLRTVulkanSetupDevice(vk) ||
		!WLRTVulkanSetupAllocator(vk) ||
		!WLRTVulkanSetupPipelineCache(vk) ||
		!WLRTVulkanSetupFrames(vk))
	{
		WLRTVulkanCleanup(vk);
		return false;
	}
	return true;
}

void WLRTVulkanCleanup(WLRTVulkanData* vk)
{
	WLRTVulkanCleanupFrames(vk);
	WLRTVulkanCleanupPipelineCache(vk);
	vmaDestroyAllocator(vk->allocator);
	vkDestroyDevice(vk->device, &vk->allocation);
	vkDestroyInstance(vk->instance, &vk->allocation);
	vk->allocator = NULL;
	vk->device    = NULL;
	vk->instance  = NULL;
}