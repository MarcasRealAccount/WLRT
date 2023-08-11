#include "Vk.h"

bool VkSetupRayTracingPipeline(VkData* vk, VkRayTracingPipelineData* rtPipeline)
{
	if (!vk || !rtPipeline) return false;

	VkRayTracingPipelineCreateInfoKHR createInfo = {
		.sType                        = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		.pNext                        = NULL,
		.flags                        = 0,
		.stageCount                   = 0,
		.pStages                      = NULL,
		.groupCount                   = 0,
		.pGroups                      = NULL,
		.maxPipelineRayRecursionDepth = 0,
		.pLibraryInfo                 = NULL,
		.pLibraryInterface            = NULL,
		.pDynamicState                = NULL,
		.layout                       = NULL,
		.basePipelineHandle           = NULL,
		.basePipelineIndex            = 0
	};
	if (!VkValidate(vk, vkCreateRayTracingPipelinesKHR(vk->device, NULL, vk->pipelineCache, 1, &createInfo, vk->allocation, &rtPipeline->handle))) return false;
	return true;
}

void VkCleanupRayTracingPipeline(VkData* vk, VkRayTracingPipelineData* rtPipeline)
{
	if (!vk || !rtPipeline) return;

	vkDestroyPipeline(vk->device, rtPipeline->handle, vk->allocation);
}