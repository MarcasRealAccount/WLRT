#pragma once

#include <stdbool.h>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

typedef void (*VkErrorCallbackFn)(int code, const char* msg);

typedef enum VkErrorCode
{
	VK_ERROR_CODE_CALL_FAILURE        = -1,
	VK_ERROR_CODE_ALLOCATION_FAILURE  = -2,
	VK_ERROR_CODE_NO_PHYSICAL_DEVICES = -3,
	VK_ERROR_CODE_INVALID_FRAME       = -4
} VkErrorCode;

typedef struct VkFrameData
{
	VkCommandPool   pool;
	VkCommandBuffer buffer;

	VkSemaphore semaphore;
	uint64_t    value;

	uint32_t                 swapchainCount;
	struct VkSwapchainData** swapchainDatas;
	VkSwapchainKHR*          swapchains;
	uint32_t*                imageIndices;
	VkImageMemoryBarrier2*   imageBarriers;
	VkSemaphoreSubmitInfo*   imageWaits;
	VkSemaphoreSubmitInfo*   renderSigs;
	VkSemaphore*             renderWaits;
	VkResult*                results;
} VkFrameData;

typedef struct VkData
{
	VkAllocationCallbacks* allocation;

	VkInstance       instance;
	VkPhysicalDevice physicalDevice;
	VkDevice         device;
	VkQueue          queue;
	VmaAllocator     allocator;
	VkPipelineCache  pipelineCache;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR    deviceRayTracingPipelineProps;
	VkPhysicalDeviceAccelerationStructurePropertiesKHR deviceAccStructureProps;
	VkPhysicalDeviceProperties2                        deviceProps;

	uint32_t     currentFrame;
	uint32_t     framesInFlight;
	uint32_t     framesCapacity;
	VkFrameData* frames;
	bool         inFrame;

	VkResult          lastResult;
	VkErrorCallbackFn errorCallback;
} VkData;

typedef struct VkSwapchainData
{
	VkData*            vk;
	struct WindowData* window;

	VkSurfaceKHR       surface;
	VkSwapchainKHR     swapchain;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR   presentMode;
	VkExtent2D         extent;
	bool               invalid;

	uint32_t     imageIndex;
	uint32_t     imageCount;
	VkImage*     images;
	VkImageView* views;
	VkSemaphore* imageAvailable;
	VkSemaphore* renderFinished;
} VkSwapchainData;

typedef struct VkAccStructBuilder
{
	VkData*     vk;
	VkQueryPool queryPool;

	VkAccelerationStructureTypeKHR       type;
	VkBuildAccelerationStructureFlagsKHR flags;

	uint64_t buildSize;
	uint64_t buildScratchSize;
	uint64_t updateScratchSize;
	bool     sizeInvalid;

	size_t        scratchBufferCapacity;
	VkBuffer      scratchBuffer;
	VmaAllocation scratchBufferA;

	uint32_t                                  firstGeometry;
	uint32_t                                  geometryCount;
	size_t                                    geometryCapacity;
	VkAccelerationStructureGeometryKHR*       geometries;
	uint32_t*                                 primitiveCounts;
	VkAccelerationStructureBuildRangeInfoKHR* ranges;
} VkAccStructBuilder;

typedef struct VkAccStruct
{
	VkData* vk;

	VkAccelerationStructureKHR handle;
	VkBuffer                   buffer;
	VmaAllocation              allocation;
} VkAccStruct;

typedef struct VkShaderData
{
	VkData* vk;

	VkShaderModule handle;

	size_t filepathLength;
	char*  filepath;

	bool modified;
} VkShaderData;

typedef struct VkRayTracingPipelineData
{
	VkData* vk;

	VkPipeline handle;
} VkRayTracingPipelineData;

const char* VkGetErrorString(int code);
const char* VkGetResultString(VkResult result);

void VkReportError(VkData* vk, int code, const char* msg);
bool VkValidate(VkData* vk, VkResult result);
bool VkValidateAllowed(VkData* vk, VkResult result, VkResult* allowed, uint32_t allowedCount);

VkFrameData* VkGetFrame(VkData* vk, uint32_t frame);
VkFrameData* VkGetCurrentFrame(VkData* vk);

bool VkBeginCmdBuffer(VkData* vk, VkCommandBuffer* buffer);
bool VkEndCmdBuffer(VkData* vk);
bool VkEndCmdBufferWait(VkData* vk);

bool VkBeginFrame(VkData* vk, VkSwapchainData** swapchains, uint32_t swapchainCount);
bool VkEndFrame(VkData* vk);

bool VkSetup(VkData* vk);
void VkCleanup(VkData* vk);
bool VkSetupFrames(VkData* vk);
void VkCleanupFrames(VkData* vk);

bool VkSetupSwapchain(VkSwapchainData* swapchain);
void VkCleanupSwapchain(VkSwapchainData* swapchain);

void VkWriteTLASInstance(void* buffer, VkAccStruct* accStruct, uint32_t index, const VkTransformMatrixKHR* transform, uint32_t customIndex, uint8_t mask, uint32_t sbtOffset, VkGeometryInstanceFlagsKHR flags);

bool VkSetupAccStructBuilder(VkAccStructBuilder* builder);
void VkCleanupAccStructBuilder(VkAccStructBuilder* builder);
void VkCleanupAccStruct(VkAccStruct* accStruct);
bool VkAccStructBuilderSetInstances(VkAccStructBuilder* builder, uint32_t geometryIndex, VkDeviceAddress deviceAddress, uint32_t count);
bool VkAccStructBuilderSetTriangles(VkAccStructBuilder* builder, uint32_t geometryIndex, VkDeviceAddress vertexAddress, VkFormat vertexFormat, uint32_t vertexStride, uint32_t maxVertex, VkDeviceAddress indexAddress, VkIndexType indexType, uint32_t triangleCount);
bool VkAccStructBuilderPrepare(VkAccStructBuilder* builder, VkAccelerationStructureTypeKHR type, VkBuildAccelerationStructureFlagsKHR flags, uint32_t geometryCount, uint32_t firstGeometry);
bool VkAccStructBuilderBuild(VkAccStructBuilder* builder, VkAccStruct* accStruct);
bool VkAccStructBuilderCompact(VkAccStructBuilder* builder, VkAccStruct* accStruct, VkAccStruct* compactAccStruct);

bool VkSetupShader(VkShaderData* shader);
void VkCleanupShader(VkShaderData* shader);

bool VkSetupRayTracingPipeline(VkRayTracingPipelineData* rtPipeline);
void VkCleanupRayTracingPipeline(VkRayTracingPipelineData* rtPipeline);