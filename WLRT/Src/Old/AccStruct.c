#include "Vk.h"

#include <stdlib.h>
#include <string.h>

void VkWriteTLASInstance(void* buffer, VkAccStruct* accStruct, uint32_t index, const VkTransformMatrixKHR* transform, uint32_t customIndex, uint8_t mask, uint32_t sbtOffset, VkGeometryInstanceFlagsKHR flags)
{
	if (!buffer || !accStruct || !accStruct->vk) return;
	VkData* vk = accStruct->vk;

	VkAccelerationStructureDeviceAddressInfoKHR asAddressInfo = {
		.sType                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.pNext                 = NULL,
		.accelerationStructure = accStruct->handle
	};

	VkAccelerationStructureInstanceKHR* instance = ((VkAccelerationStructureInstanceKHR*) buffer) + index;

	instance->transform                              = *transform;
	instance->instanceCustomIndex                    = customIndex;
	instance->mask                                   = mask;
	instance->instanceShaderBindingTableRecordOffset = sbtOffset;
	instance->flags                                  = flags;
	instance->accelerationStructureReference         = vkGetAccelerationStructureDeviceAddressKHR(vk->device, &asAddressInfo);
}

bool VkSetupAccStructBuilder(VkAccStructBuilder* builder)
{
	if (!builder || !builder->vk) return false;
	VkData* vk = builder->vk;

	VkQueryPoolCreateInfo createInfo = {
		.sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
		.pNext              = NULL,
		.flags              = 0,
		.queryType          = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
		.queryCount         = 1,
		.pipelineStatistics = 0
	};
	if (!VkValidate(vk, vkCreateQueryPool(vk->device, &createInfo, vk->allocation, &builder->queryPool))) return false;
	builder->type                  = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	builder->flags                 = 0;
	builder->buildSize             = 0;
	builder->buildScratchSize      = 0;
	builder->updateScratchSize     = 0;
	builder->sizeInvalid           = false;
	builder->scratchBufferCapacity = 0;
	builder->scratchBuffer         = NULL;
	builder->scratchBufferA        = NULL;
	builder->firstGeometry         = 0;
	builder->geometryCount         = 0;
	builder->geometryCapacity      = 0;
	builder->geometries            = NULL;
	builder->primitiveCounts       = NULL;
	builder->ranges                = NULL;
	return true;
}

void VkCleanupAccStructBuilder(VkAccStructBuilder* builder)
{
	if (!builder || !builder->vk) return;
	VkData* vk = builder->vk;

	free(builder->geometries);
	free(builder->primitiveCounts);
	free(builder->ranges);
	vmaDestroyBuffer(vk->allocator, builder->scratchBuffer, builder->scratchBufferA);
	vkDestroyQueryPool(vk->device, builder->queryPool, vk->allocation);
	builder->queryPool             = NULL;
	builder->scratchBufferCapacity = 0;
	builder->scratchBuffer         = NULL;
	builder->scratchBufferA        = NULL;
	builder->geometryCapacity      = 0;
	builder->geometries            = NULL;
	builder->primitiveCounts       = NULL;
	builder->ranges                = NULL;
}

void VkCleanupAccStruct(VkAccStruct* accStruct)
{
	if (!accStruct || !accStruct->vk) return;
	VkData* vk = accStruct->vk;

	vmaDestroyBuffer(vk->allocator, accStruct->buffer, accStruct->allocation);
	vkDestroyAccelerationStructureKHR(vk->device, accStruct->handle, vk->allocation);
	accStruct->handle     = NULL;
	accStruct->buffer     = NULL;
	accStruct->allocation = NULL;
}

static bool VkAccStructBuilderEnsureGeometries(VkData* vk, VkAccStructBuilder* builder, uint32_t geometryIndex)
{
	if (geometryIndex >= builder->geometryCapacity)
	{
		size_t newCapacity = geometryIndex;
		newCapacity       |= newCapacity >> 1;
		newCapacity       |= newCapacity >> 2;
		newCapacity       |= newCapacity >> 4;
		newCapacity       |= newCapacity >> 8;
		newCapacity       |= newCapacity >> 16;
		newCapacity       |= newCapacity >> 32;
		++newCapacity;

		VkAccelerationStructureGeometryKHR*       newGeometry        = (VkAccelerationStructureGeometryKHR*) malloc(newCapacity * sizeof(VkAccelerationStructureGeometryKHR));
		uint32_t*                                 newPrimitiveCounts = (uint32_t*) malloc(newCapacity * sizeof(uint32_t));
		VkAccelerationStructureBuildRangeInfoKHR* newRanges          = (VkAccelerationStructureBuildRangeInfoKHR*) malloc(newCapacity * sizeof(VkAccelerationStructureBuildRangeInfoKHR));
		if (!newGeometry || !newPrimitiveCounts || !newRanges)
		{
			free(newGeometry);
			free(newPrimitiveCounts);
			free(newRanges);
			VkReportError(vk, VK_ERROR_CODE_ALLOCATION_FAILURE, "Failed to allocate acceleration structure builder buffers");
			return false;
		}
		if (builder->geometries)
		{
			memcpy(newGeometry, builder->geometries, builder->geometryCapacity * sizeof(VkAccelerationStructureGeometryKHR));
			memset(newGeometry + builder->geometryCapacity, 0, (newCapacity - builder->geometryCapacity) * sizeof(VkAccelerationStructureGeometryKHR));
			free(builder->geometries);
		}
		else
		{
			memset(newGeometry, 0, newCapacity * sizeof(VkAccelerationStructureGeometryKHR));
		}
		if (builder->primitiveCounts)
		{
			memcpy(newPrimitiveCounts, builder->primitiveCounts, builder->geometryCapacity * sizeof(uint32_t));
			memset(newPrimitiveCounts + builder->geometryCapacity, 0, (newCapacity - builder->geometryCapacity) * sizeof(uint32_t));
			free(builder->primitiveCounts);
		}
		else
		{
			memset(newPrimitiveCounts, 0, newCapacity * sizeof(uint32_t));
		}
		if (builder->ranges)
		{
			memcpy(newRanges, builder->ranges, builder->geometryCapacity * sizeof(VkAccelerationStructureBuildRangeInfoKHR));
			memset(newRanges + builder->geometryCapacity, 0, (newCapacity - builder->geometryCapacity) * sizeof(VkAccelerationStructureBuildRangeInfoKHR));
			free(builder->ranges);
		}
		else
		{
			memset(newRanges, 0, newCapacity * sizeof(VkAccelerationStructureBuildRangeInfoKHR));
		}
		builder->geometryCapacity = newCapacity;
		builder->geometries       = newGeometry;
		builder->primitiveCounts  = newPrimitiveCounts;
		builder->ranges           = newRanges;
	}
	return true;
}

bool VkAccStructBuilderSetInstances(VkAccStructBuilder* builder, uint32_t geometryIndex, VkDeviceAddress deviceAddress, uint32_t count)
{
	if (!builder || !builder->vk) return false;
	VkData* vk = builder->vk;
	if (!VkAccStructBuilderEnsureGeometries(vk, builder, geometryIndex)) return false;

	VkAccelerationStructureGeometryKHR* instances    = builder->geometries + geometryIndex;
	instances->sType                                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	instances->pNext                                 = NULL;
	instances->geometryType                          = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	instances->geometry.instances.sType              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	instances->geometry.instances.pNext              = NULL;
	instances->geometry.instances.arrayOfPointers    = VK_FALSE;
	instances->geometry.instances.data.deviceAddress = deviceAddress;
	instances->flags                                 = 0;

	VkAccelerationStructureBuildRangeInfoKHR* range = builder->ranges + geometryIndex;
	range->primitiveCount                           = count;

	builder->primitiveCounts[geometryIndex] = count;
	return true;
}

bool VkAccStructBuilderSetTriangles(VkAccStructBuilder* builder, uint32_t geometryIndex, VkDeviceAddress vertexAddress, VkFormat vertexFormat, uint32_t vertexStride, uint32_t maxVertex, VkDeviceAddress indexAddress, VkIndexType indexType, uint32_t triangleCount)
{
	if (!builder || !builder->vk) return false;
	VkData* vk = builder->vk;
	if (!VkAccStructBuilderEnsureGeometries(vk, builder, geometryIndex)) return false;

	VkAccelerationStructureGeometryKHR* triangles             = builder->geometries + geometryIndex;
	triangles->sType                                          = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	triangles->pNext                                          = NULL;
	triangles->geometryType                                   = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	triangles->geometry.triangles.sType                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	triangles->geometry.triangles.pNext                       = NULL;
	triangles->geometry.triangles.vertexFormat                = vertexFormat;
	triangles->geometry.triangles.vertexData.deviceAddress    = vertexAddress;
	triangles->geometry.triangles.vertexStride                = vertexStride;
	triangles->geometry.triangles.maxVertex                   = maxVertex;
	triangles->geometry.triangles.indexType                   = indexType;
	triangles->geometry.triangles.indexData.deviceAddress     = indexAddress;
	triangles->geometry.triangles.transformData.deviceAddress = 0;
	triangles->flags                                          = 0;

	VkAccelerationStructureBuildRangeInfoKHR* range = builder->ranges + geometryIndex;
	range->primitiveCount                           = triangleCount;

	builder->primitiveCounts[geometryIndex] = triangleCount;
	return true;
}

bool VkAccStructBuilderPrepare(VkAccStructBuilder* builder, VkAccelerationStructureTypeKHR type, VkBuildAccelerationStructureFlagsKHR flags, uint32_t geometryCount, uint32_t firstGeometry)
{
	if (!builder) return false;
	builder->type          = type;
	builder->flags         = flags;
	builder->geometryCount = geometryCount;
	builder->firstGeometry = firstGeometry;
	builder->sizeInvalid   = true;
	return true;
}

static bool VkAccStructBuilderEnsureSizes(VkData* vk, VkAccStructBuilder* builder)
{
	if (!builder->sizeInvalid) return true;

	VkAccelerationStructureBuildSizesInfoKHR sizes = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		.pNext = NULL
	};
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {
		.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext                     = NULL,
		.type                      = builder->type,
		.flags                     = builder->flags,
		.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure  = NULL,
		.dstAccelerationStructure  = NULL,
		.geometryCount             = builder->geometryCount,
		.pGeometries               = builder->geometries + builder->firstGeometry,
		.ppGeometries              = NULL,
		.scratchData.deviceAddress = 0
	};
	vkGetAccelerationStructureBuildSizesKHR(vk->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, builder->primitiveCounts, &sizes);
	builder->buildSize         = sizes.accelerationStructureSize;
	builder->buildScratchSize  = sizes.buildScratchSize;
	builder->updateScratchSize = sizes.updateScratchSize;
	builder->sizeInvalid       = false;
	return true;
}

static bool VkAccStructBuilderEnsureScratchSize(VkAccStructBuilder* builder, size_t scratchSize)
{
	if (!builder || !builder->vk) return false;
	VkData* vk = builder->vk;

	if (scratchSize >= builder->scratchBufferCapacity)
	{
		size_t newCapacity = scratchSize;

		vmaDestroyBuffer(vk->allocator, builder->scratchBuffer, builder->scratchBufferA);
		VkBufferCreateInfo createInfo = {
			.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext                 = NULL,
			.flags                 = 0,
			.size                  = newCapacity,
			.usage                 = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices   = NULL
		};
		VmaAllocationCreateInfo allocInfo = {
			.flags          = 0,
			.usage          = VMA_MEMORY_USAGE_CPU_ONLY,
			.requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
			.pool           = NULL,
			.pUserData      = NULL,
			.priority       = 0.0f
		};
		if (!VkValidate(vk, vmaCreateBufferWithAlignment(vk->allocator, &createInfo, &allocInfo, vk->deviceAccStructureProps.minAccelerationStructureScratchOffsetAlignment, &builder->scratchBuffer, &builder->scratchBufferA, NULL)))
		{
			builder->scratchBufferCapacity = 0;
			return false;
		}
		builder->scratchBufferCapacity = newCapacity;
	}
	return true;
}

bool VkAccStructBuilderBuild(VkAccStructBuilder* builder, VkAccStruct* accStruct)
{
	if (!builder || !builder->vk || !accStruct) return false;
	VkData* vk = builder->vk;

	VkAccStructBuilderEnsureSizes(vk, builder);

	VkBufferCreateInfo bCreateInfo = {
		.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.size                  = builder->buildSize,
		.usage                 = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL
	};
	VmaAllocationCreateInfo bAllocInfo = {
		.flags          = 0,
		.usage          = VMA_MEMORY_USAGE_CPU_ONLY,
		.requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool           = NULL,
		.pUserData      = NULL,
		.priority       = 0.0f
	};
	if (!VkValidate(vk, vmaCreateBufferWithAlignment(vk->allocator, &bCreateInfo, &bAllocInfo, vk->deviceAccStructureProps.minAccelerationStructureScratchOffsetAlignment, &accStruct->buffer, &accStruct->allocation, NULL)))
		return false;

	VkAccelerationStructureCreateInfoKHR aCreateInfo = {
		.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext         = NULL,
		.createFlags   = 0,
		.buffer        = accStruct->buffer,
		.offset        = 0,
		.size          = builder->buildSize,
		.type          = builder->type,
		.deviceAddress = 0
	};
	if (!VkValidate(vk, vkCreateAccelerationStructureKHR(vk->device, &aCreateInfo, vk->allocation, &accStruct->handle)))
	{
		VkCleanupAccStruct(accStruct);
		return false;
	}

	if (!VkAccStructBuilderEnsureScratchSize(builder, builder->buildScratchSize))
	{
		VkCleanupAccStruct(accStruct);
		return false;
	}

	bool compact = builder->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;

	VkCommandBuffer buffer = NULL;
	if (!VkBeginCmdBuffer(vk, &buffer))
	{
		VkCleanupAccStruct(accStruct);
		return false;
	}

	if (compact) vkCmdResetQueryPool(buffer, builder->queryPool, 0, 1);

	VkBufferDeviceAddressInfo scratchBufferAddressInfo = {
		.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext  = NULL,
		.buffer = builder->scratchBuffer
	};

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {
		.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext                     = NULL,
		.type                      = builder->type,
		.flags                     = builder->flags,
		.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure  = NULL,
		.dstAccelerationStructure  = accStruct->handle,
		.geometryCount             = builder->geometryCount,
		.pGeometries               = builder->geometries + builder->firstGeometry,
		.ppGeometries              = NULL,
		.scratchData.deviceAddress = vkGetBufferDeviceAddress(vk->device, &scratchBufferAddressInfo)
	};
	vkCmdBuildAccelerationStructuresKHR(buffer, 1, &buildInfo, &builder->ranges);

	if (compact)
	{
		VkMemoryBarrier2 memoryBarrier = {
			.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
			.pNext         = NULL,
			.srcStageMask  = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask  = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			.dstAccessMask = VK_ACCESS_2_NONE
		};
		VkDependencyInfo dependencyInfo = {
			.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext                    = NULL,
			.dependencyFlags          = 0,
			.memoryBarrierCount       = 1,
			.pMemoryBarriers          = &memoryBarrier,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers    = NULL,
			.imageMemoryBarrierCount  = 0,
			.pImageMemoryBarriers     = NULL
		};
		vkCmdPipelineBarrier2(buffer, &dependencyInfo);
		vkCmdWriteAccelerationStructuresPropertiesKHR(buffer, 1, &accStruct->handle, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, builder->queryPool, 0);
	}

	if (!VkEndCmdBufferWait(vk))
	{
		VkCleanupAccStruct(accStruct);
		return false;
	}
	return true;
}

bool VkAccStructBuilderCompact(VkAccStructBuilder* builder, VkAccStruct* accStruct, VkAccStruct* compactAccStruct)
{
	if (!builder || !builder->vk || !accStruct || !compactAccStruct) return false;
	VkData* vk = builder->vk;

	VkDeviceSize size = 0;
	if (!VkValidate(vk, vkGetQueryPoolResults(vk->device, builder->queryPool, 0, 1, sizeof(size), &size, sizeof(size), VK_QUERY_RESULT_64_BIT))) return false;

	VkBufferCreateInfo bCreateInfo = {
		.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.size                  = size,
		.usage                 = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL
	};
	VmaAllocationCreateInfo bAllocInfo = {
		.flags          = 0,
		.usage          = VMA_MEMORY_USAGE_CPU_ONLY,
		.requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool           = NULL,
		.pUserData      = NULL,
		.priority       = 0.0f
	};
	if (!VkValidate(vk, vmaCreateBufferWithAlignment(vk->allocator, &bCreateInfo, &bAllocInfo, vk->deviceAccStructureProps.minAccelerationStructureScratchOffsetAlignment, &compactAccStruct->buffer, &compactAccStruct->allocation, NULL)))
		return false;

	VkAccelerationStructureCreateInfoKHR aCreateInfo = {
		.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext         = NULL,
		.createFlags   = 0,
		.buffer        = compactAccStruct->buffer,
		.offset        = 0,
		.size          = size,
		.type          = builder->type,
		.deviceAddress = 0
	};
	if (!VkValidate(vk, vkCreateAccelerationStructureKHR(vk->device, &aCreateInfo, vk->allocation, &compactAccStruct->handle)))
	{
		VkCleanupAccStruct(compactAccStruct);
		return false;
	}

	VkCommandBuffer buffer = NULL;
	if (!VkBeginCmdBuffer(vk, &buffer))
	{
		VkCleanupAccStruct(compactAccStruct);
		return false;
	}

	VkCopyAccelerationStructureInfoKHR copyInfo = {
		.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
		.pNext = NULL,
		.src   = accStruct->handle,
		.dst   = compactAccStruct->handle,
		.mode  = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR
	};
	vkCmdCopyAccelerationStructureKHR(buffer, &copyInfo);

	if (!VkEndCmdBufferWait(vk))
	{
		VkCleanupAccStruct(compactAccStruct);
		return false;
	}
	return true;
}