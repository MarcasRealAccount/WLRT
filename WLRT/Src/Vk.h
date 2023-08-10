#pragma once

#include <stdbool.h>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

typedef void (*VkErrorCallbackFn)(int code, const char* msg);

typedef enum VkErrorCode
{
	VK_ERROR_CODE_CALL_FAILURE        = -1,
	VK_ERROR_CODE_ALLOCATION_FAILURE  = -2,
	VK_ERROR_CODE_NO_PHYSICAL_DEVICES = -3
};

typedef struct VkData
{
	VkAllocationCallbacks* allocation;

	VkInstance       instance;
	VkPhysicalDevice physicalDevice;
	VkDevice         device;
	VkQueue          queue;
	VmaAllocator     allocator;

	VkPhysicalDeviceAccelerationStructurePropertiesKHR deviceAccStructureProps;
	VkPhysicalDeviceProperties2                        deviceProps;

	VkResult          lastResult;
	VkErrorCallbackFn errorCallback;
} VkData;

const char* VkGetErrorString(int code);
const char* VkGetResultString(VkResult result);

void VkReportError(VkData* vk, int code, const char* msg);
bool VkValidate(VkData* vk, VkResult result);
bool VkValidateAllowed(VkData* vk, VkResult result, VkResult* allowed, uint32_t allowedCount);

bool VkSetup(VkData* vk);
void VkCleanup(VkData* vk);