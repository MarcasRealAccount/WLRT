#include "Vk.h"

bool VkSetupRayTracingPipeline(VkRayTracingPipelineData* rtPipeline)
{
	if (!rtPipeline || !rtPipeline->vk) return false;
	VkData* vk = rtPipeline->vk;

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

void VkCleanupRayTracingPipeline(VkRayTracingPipelineData* rtPipeline)
{
	if (!rtPipeline || !rtPipeline->vk) return;
	VkData* vk = rtPipeline->vk;

	vkDestroyPipeline(vk->device, rtPipeline->handle, vk->allocation);
}