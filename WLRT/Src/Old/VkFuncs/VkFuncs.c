#include "VkFuncs.h"

void VkLoadFuncs(VkInstance instance, VkDevice device)
{
	VkLoadAccelerationStructureFuncs(instance, device);
	VkLoadRayTracingFuncs(instance, device);
}