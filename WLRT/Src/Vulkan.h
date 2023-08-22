#pragma once

#include "DynArray.h"
#include "FileSystem.h"
#include "Logging.h"

#include <stdbool.h>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

typedef enum WLRTVulkanErrorCode
{
	WLRT_VULKAN_ERROR_CODE_CALL_FAILURE        = -1,
	WLRT_VULKAN_ERROR_CODE_ALLOCATION_FAILURE  = -2,
	WLRT_VULKAN_ERROR_CODE_NO_PHYSICAL_DEVICES = -3,
	WLRT_VULKAN_ERROR_CODE_INVALID_FRAME       = -4
} WLRTVulkanErrorCode;

typedef struct WLRTVulkanDeviceProperties
{
	VkPhysicalDeviceProperties2                        properties2;
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR    rayTracingPipelineProperties;
	VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties;
} WLRTVulkanDeviceProperties;

typedef struct WLRTVulkanFrameData
{
	struct WLRTVulkanData* vk;

	VkCommandPool   pool;
	VkCommandBuffer buffer;

	VkSemaphore semaphore;
	uint64_t    value;

	WLRTMultiDynArray renderData;
} WLRTVulkanFrameData;

typedef struct WLRTVulkanData
{
	VkAllocationCallbacks allocation;

	VkInstance       instance;
	VkPhysicalDevice physicalDevice;
	VkDevice         device;
	VkQueue          queue;
	VmaAllocator     allocator;
	VkPipelineCache  pipelineCache;

	uint32_t     currentFrame;
	uint32_t     framesInFlight;
	WLRTDynArray frames;
	bool         inFrame;

	VkResult       lastResult;
	WLRTLoggerData logger;

	WLRTVulkanDeviceProperties deviceProps;
} WLRTVulkanData;

typedef struct WLRTVulkanSwapchainData
{
	WLRTVulkanData*        vk;
	struct WLRTWindowData* window;

	VkSurfaceKHR       surface;
	VkSwapchainKHR     swapchain;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR   presentMode;
	VkExtent2D         extent;
	bool               invalid;

	uint32_t          imageIndex;
	WLRTMultiDynArray images;
} WLRTVulkanSwapchainData;

typedef struct WLRTVulkanShaderData
{
	WLRTVulkanData* vk;

	VkShaderModule handle;

	WLRTPath file;
	uint64_t watchId;

	bool modified;
} WLRTVulkanShaderData;

typedef struct WLRTVulkanAccelerationStructureData
{
	WLRTVulkanData* vk;

	VkAccelerationStructureKHR handle;
	VkBuffer                   buffer;
	VmaAllocation              allocation;
} WLRTVulkanAccelerationStructureData;

typedef struct WLRTVulkanAccelerationStructureBuilder
{
	WLRTVulkanData* vk;
	VkQueryPool     queryPool;

	VkAccelerationStructureTypeKHR       type;
	VkBuildAccelerationStructureFlagsKHR flags;

	uint64_t buildSize;
	uint64_t buildScratchSize;
	uint64_t updateScratchSize;
	bool     sizeInvalid;

	size_t        scratchBufferCapacity;
	VkBuffer      scratchBuffer;
	VmaAllocation scratchBufferA;

	uint32_t          firstGeometry;
	uint32_t          geometryCount;
	WLRTMultiDynArray geometries;
} WLRTVulkanAccelerationStructureBuilder;

bool WLRTVulkanValidate(WLRTVulkanData* vk, VkResult result);
bool WLRTVulkanValidateAllowed(WLRTVulkanData* vk, VkResult result, VkResult* allowed, uint32_t allowedCount);

WLRTVulkanData* WLRTVulkanGetFrame(WLRTVulkanData* vk, uint32_t frame);
WLRTVulkanData* WLRTVulkanGetCurrentFrame(WLRTVulkanData* vk);

bool WLRTVulkanBeginFrame(WLRTVulkanData* vk, WLRTVulkanSwapchainData** swapchains, uint32_t swapchainCount);
void WLRTVulkanEndFrame(WLRTVulkanData* vk);

bool WLRTVulkanSetup(WLRTVulkanData* vk);
void WLRTVulkanCleanup(WLRTVulkanData* vk);

bool WLRTVulkanSwapchainSetup(WLRTVulkanSwapchainData* swapchain);
void WLRTVulkanSwapchainCleanup(WLRTVulkanSwapchainData* swapchain);

bool WLRTVulkanShaderSetup(WLRTVulkanShaderData* shader);
void WLRTVulkanShaderCleanup(WLRTVulkanShaderData* shader);

void WLRTVulkanAccelerationStructureCleanup(WLRTVulkanAccelerationStructureData* accStruct);

bool WLRTVulkanAccelerationStructureBuilderSetup(WLRTVulkanAccelerationStructureBuilder* builder);
void WLRTVulkanAccelerationStructureBuilderCleanup(WLRTVulkanAccelerationStructureBuilder* builder);