#include <vulkan/vulkan.h>

static PFN_vkCreateAccelerationStructureKHR              pfnVkCreateAccelerationStructureKHR              = NULL;
static PFN_vkDestroyAccelerationStructureKHR             pfnVkDestroyAccelerationStructureKHR             = NULL;
static PFN_vkCmdBuildAccelerationStructuresKHR           pfnVkCmdBuildAccelerationStructuresKHR           = NULL;
static PFN_vkCmdCopyAccelerationStructureKHR             pfnVkCmdCopyAccelerationStructureKHR             = NULL;
static PFN_vkCmdWriteAccelerationStructuresPropertiesKHR pfnVkCmdWriteAccelerationStructuresPropertiesKHR = NULL;
static PFN_vkGetAccelerationStructureBuildSizesKHR       pfnVkGetAccelerationStructureBuildSizesKHR       = NULL;
static PFN_vkGetAccelerationStructureDeviceAddressKHR    pfnVkGetAccelerationStructureDeviceAddressKHR    = NULL;

void VkLoadAccelerationStructureFuncs(VkInstance instance, VkDevice device)
{
	(void) instance;
	if (device)
	{
		pfnVkCreateAccelerationStructureKHR              = (PFN_vkCreateAccelerationStructureKHR) vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
		pfnVkDestroyAccelerationStructureKHR             = (PFN_vkDestroyAccelerationStructureKHR) vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
		pfnVkCmdBuildAccelerationStructuresKHR           = (PFN_vkCmdBuildAccelerationStructuresKHR) vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
		pfnVkCmdCopyAccelerationStructureKHR             = (PFN_vkCmdCopyAccelerationStructureKHR) vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR");
		pfnVkCmdWriteAccelerationStructuresPropertiesKHR = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR) vkGetDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesKHR");
		pfnVkGetAccelerationStructureBuildSizesKHR       = (PFN_vkGetAccelerationStructureBuildSizesKHR) vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR");
		pfnVkGetAccelerationStructureDeviceAddressKHR    = (PFN_vkGetAccelerationStructureDeviceAddressKHR) vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR");
	}
}

VkResult vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure)
{
	if (!pfnVkCreateAccelerationStructureKHR) return VK_ERROR_EXTENSION_NOT_PRESENT;
	return pfnVkCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

void vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator)
{
	if (pfnVkDestroyAccelerationStructureKHR)
		pfnVkDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

void vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
	if (pfnVkCmdBuildAccelerationStructuresKHR)
		pfnVkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

void vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo)
{
	if (pfnVkCmdCopyAccelerationStructureKHR)
		pfnVkCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}

void vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery)
{
	if (pfnVkCmdWriteAccelerationStructuresPropertiesKHR)
		pfnVkCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

void vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo, const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo)
{
	if (pfnVkGetAccelerationStructureBuildSizesKHR)
		pfnVkGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

VkDeviceAddress vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo)
{
	if (!pfnVkGetAccelerationStructureDeviceAddressKHR) return 0;
	return pfnVkGetAccelerationStructureDeviceAddressKHR(device, pInfo);
}