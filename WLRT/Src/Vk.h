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
};

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

const char* VkGetErrorString(int code);
const char* VkGetResultString(VkResult result);

void VkReportError(VkData* vk, int code, const char* msg);
bool VkValidate(VkData* vk, VkResult result);
bool VkValidateAllowed(VkData* vk, VkResult result, VkResult* allowed, uint32_t allowedCount);

VkFrameData* VkGetFrame(VkData* vk, uint32_t frame);
VkFrameData* VkGetCurrentFrame(VkData* vk);

bool VkBeginCmdBuffer(VkData* vk, VkCommandBuffer* buffer);
bool VkEndCmdBuffer(VkData* vk);

bool VkBeginFrame(VkData* vk, VkSwapchainData** swapchains, uint32_t swapchainCount);
bool VkEndFrame(VkData* vk);

bool VkSetup(VkData* vk);
void VkCleanup(VkData* vk);
bool VkSetupFrames(VkData* vk);
void VkCleanupFrames(VkData* vk);

bool VkSetupSwapchain(VkData* vk, VkSwapchainData* swapchain);
void VkCleanupSwapchain(VkData* vk, VkSwapchainData* swapchain);