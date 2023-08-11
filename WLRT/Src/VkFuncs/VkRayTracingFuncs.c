#include <vulkan/vulkan.h>

static PFN_vkCreateRayTracingPipelinesKHR pfnVkCreateRayTracingPipelinesKHR = NULL;

void VkLoadRayTracingFuncs(VkInstance instance, VkDevice device)
{
	pfnVkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR) vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR");
}

VkResult vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
	if (!pfnVkCreateRayTracingPipelinesKHR) return VK_ERROR_EXTENSION_NOT_PRESENT;
	return pfnVkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}