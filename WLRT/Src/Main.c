#include "Exit.h"
#include "Vk.h"
#include "VkFuncs/VkFuncs.h"
#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void GLFWErrCB(int code, const char* msg)
{
	printf("GLFW ERROR (%d): %s\n", code, msg);
}

static void VKErrCB(int code, const char* msg)
{
	printf("VK ERROR (%d %s): %s\n", code, VkGetErrorString(code), msg);
}

static bool CreateBLAS(VkAccStructBuilder* builder, VkAccStruct* blas)
{
	typedef struct Vertex
	{
		float x, y, z, w;
	} Vertex;

	typedef uint32_t Index;

	VkData* vk = builder->vk;

	Vertex vertices[] = {
		{0.5f,  0.2f, 0.0f, 0.0f},
		{ 0.2f, 0.8f, 0.0f, 0.0f},
		{ 0.8f, 0.8f, 0.0f, 0.0f}
	};
	Index indices[] = { 0, 1, 2 };

	VkBuffer      vertexBuffer  = NULL;
	VkBuffer      indexBuffer   = NULL;
	VmaAllocation vertexBufferA = NULL;
	VmaAllocation indexBufferA  = NULL;

	VkBufferCreateInfo vbCreateInfo = {
		.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.size                  = sizeof(vertices),
		.usage                 = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL
	};
	VmaAllocationCreateInfo vbAllocInfo = {
		.flags          = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage          = VMA_MEMORY_USAGE_CPU_ONLY,
		.requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool           = NULL,
		.pUserData      = NULL,
		.priority       = 0.0f
	};
	VkBufferCreateInfo ibCreateInfo = {
		.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.size                  = sizeof(indices),
		.usage                 = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL
	};
	VmaAllocationCreateInfo ibAllocInfo = {
		.flags          = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage          = VMA_MEMORY_USAGE_CPU_ONLY,
		.requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool           = NULL,
		.pUserData      = NULL,
		.priority       = 0.0f
	};
	if (!VkValidate(vk, vmaCreateBuffer(vk->allocator, &vbCreateInfo, &vbAllocInfo, &vertexBuffer, &vertexBufferA, NULL)) ||
		!VkValidate(vk, vmaCreateBuffer(vk->allocator, &ibCreateInfo, &ibAllocInfo, &indexBuffer, &indexBufferA, NULL)))
	{
		vmaDestroyBuffer(vk->allocator, vertexBuffer, vertexBufferA);
		vmaDestroyBuffer(vk->allocator, indexBuffer, indexBufferA);
		return false;
	}

	void* vertexData = NULL;
	void* indexData  = NULL;
	if (!VkValidate(vk, vmaMapMemory(vk->allocator, vertexBufferA, &vertexData)) ||
		!VkValidate(vk, vmaMapMemory(vk->allocator, indexBufferA, &indexData)))
	{
		vmaUnmapMemory(vk->allocator, vertexBufferA);
		vmaUnmapMemory(vk->allocator, indexBufferA);
		vmaDestroyBuffer(vk->allocator, vertexBuffer, vertexBufferA);
		vmaDestroyBuffer(vk->allocator, indexBuffer, indexBufferA);
		return false;
	}
	memcpy(vertexData, vertices, sizeof(vertices));
	memcpy(indexData, indices, sizeof(indices));
	vmaUnmapMemory(vk->allocator, vertexBufferA);
	vmaUnmapMemory(vk->allocator, indexBufferA);

	VkBufferDeviceAddressInfo vbAddressInfo = {
		.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext  = NULL,
		.buffer = vertexBuffer
	};
	VkBufferDeviceAddressInfo ibAddressInfo = {
		.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext  = NULL,
		.buffer = indexBuffer
	};

	VkAccStruct uncompressed;
	memset(&uncompressed, 0, sizeof(uncompressed));
	if (!VkAccStructBuilderSetTriangles(builder, 0, vkGetBufferDeviceAddress(vk->device, &vbAddressInfo), VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vertex), sizeof(vertices) / sizeof(*vertices), vkGetBufferDeviceAddress(vk->device, &ibAddressInfo), VK_INDEX_TYPE_UINT32, sizeof(indices) / sizeof(*indices)) ||
		!VkAccStructBuilderPrepare(builder, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR, 1, 0) ||
		!VkAccStructBuilderBuild(builder, &uncompressed) ||
		!VkAccStructBuilderCompact(builder, &uncompressed, blas))
	{
		VkCleanupAccStruct(&uncompressed);
		vmaDestroyBuffer(vk->allocator, vertexBuffer, vertexBufferA);
		vmaDestroyBuffer(vk->allocator, indexBuffer, indexBufferA);
		return false;
	}

	VkCleanupAccStruct(&uncompressed);
	vmaDestroyBuffer(vk->allocator, vertexBuffer, vertexBufferA);
	vmaDestroyBuffer(vk->allocator, indexBuffer, indexBufferA);
	return true;
}

static bool CreateTLAS(VkAccStructBuilder* builder, VkAccStruct* blas, VkAccStruct* tlas)
{
	VkData* vk = builder->vk;

	VkBuffer      instancesBuffer  = NULL;
	VmaAllocation instancesBufferA = NULL;
	void*         instancesData    = NULL;

	VkBufferCreateInfo iCreateInfo = {
		.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.size                  = 1 * sizeof(VkAccelerationStructureInstanceKHR),
		.usage                 = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL
	};
	VmaAllocationCreateInfo iAllocInfo = {
		.flags          = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage          = VMA_MEMORY_USAGE_CPU_ONLY,
		.requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool           = NULL,
		.pUserData      = NULL,
		.priority       = 0.0f
	};
	if (!VkValidate(vk, vmaCreateBuffer(vk->allocator, &iCreateInfo, &iAllocInfo, &instancesBuffer, &instancesBufferA, NULL)) ||
		!VkValidate(vk, vmaMapMemory(vk->allocator, instancesBufferA, &instancesData)))
	{
		vmaDestroyBuffer(vk->allocator, instancesBuffer, instancesBufferA);
		return false;
	}
	VkTransformMatrixKHR identityMatrix = {
		.matrix = {{ 1.0f, 0.0f, 0.0f, 0.0f },
                   { 0.0f, 1.0f, 0.0f, 0.0f },
                   { 0.0f, 0.0f, 1.0f, 0.0f }}
	};
	VkWriteTLASInstance(instancesData, blas, 0, &identityMatrix, 0, 0xFF, 0, 0);
	vmaUnmapMemory(vk->allocator, instancesBufferA);

	VkBufferDeviceAddressInfo iAddressInfo = {
		.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext  = NULL,
		.buffer = instancesBuffer
	};
	if (!VkAccStructBuilderSetInstances(builder, 0, vkGetBufferDeviceAddress(vk->device, &iAddressInfo), 1) ||
		!VkAccStructBuilderPrepare(builder, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, 1, 0) ||
		!VkAccStructBuilderBuild(builder, tlas))
	{
		vmaDestroyBuffer(vk->allocator, instancesBuffer, instancesBufferA);
		return false;
	}

	vmaDestroyBuffer(vk->allocator, instancesBuffer, instancesBufferA);
	return true;
}

static bool CreateAS(VkAccStruct* blas, VkAccStruct* tlas)
{
	VkAccStructBuilder builder;
	memset(&builder, 0, sizeof(builder));
	builder.vk = blas->vk;
	if (!VkSetupAccStructBuilder(&builder)) return false;

	if (!CreateBLAS(&builder, blas))
	{
		VkCleanupAccStructBuilder(&builder);
		return false;
	}

	if (!CreateTLAS(&builder, blas, tlas))
	{
		VkCleanupAccStruct(blas);
		VkCleanupAccStructBuilder(&builder);
		return false;
	}

	VkCleanupAccStructBuilder(&builder);
	return true;
}

static void GLFWOnExit(void* data)
{
	(void) data;
	glfwTerminate();
}

typedef struct AppData
{
	VkData*          vk;
	WindowData*      window;
	VkSwapchainData* vkSwapchain;

	size_t       accStructCount;
	VkAccStruct* accStructs;

	size_t        shaderCount;
	VkShaderData* shaders;

	VkRayTracingPipelineData* rtPipeline;
} AppData;

static void AppOnExit(void* data)
{
	AppData* appData = (AppData*) data;
	if (appData->vk)
		vkDeviceWaitIdle(appData->vk->device);

	VkCleanupRayTracingPipeline(appData->rtPipeline);
	free(appData->rtPipeline);
	if (appData->shaders)
	{
		for (size_t i = 0; i < appData->shaderCount; ++i)
			VkCleanupShader(appData->shaders + i);
		free(appData->shaders);
	}
	if (appData->accStructs)
	{
		for (size_t i = 0; i < appData->accStructCount; ++i)
			VkCleanupAccStruct(appData->accStructs + i);
		free(appData->accStructs);
	}
	VkCleanupSwapchain(appData->vkSwapchain);
	free(appData->vkSwapchain);
	WLRTDestroyWindow(appData->window);
	free(appData->window);
	VkCleanup(appData->vk);
	free(appData->vk);
	free(appData);
}

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;
	ExitAssert(ExitSetup(), 1);

	glfwSetErrorCallback(&GLFWErrCB);
	ExitAssert(glfwInit(), 1);
	ExitRegister(&GLFWOnExit, NULL);

	AppData* appData = (AppData*) calloc(1, sizeof(AppData));
	ExitAssert(appData != NULL, 1);
	ExitRegister(&AppOnExit, appData);

	appData->vk = (VkData*) calloc(1, sizeof(VkData));
	ExitAssert(appData->vk != NULL, 1);
	appData->vk->framesInFlight = 2;
	appData->vk->errorCallback  = &VKErrCB;
	ExitAssert(VkSetup(appData->vk), 1);
	VkLoadFuncs(appData->vk->instance, appData->vk->device);

	appData->window = (WindowData*) calloc(1, sizeof(WindowData));
	ExitAssert(appData->window != NULL, 1);
	appData->window->x      = 1 << 31;
	appData->window->y      = 1 << 31;
	appData->window->width  = 1280;
	appData->window->height = 720;
	ExitAssert(WLRTCreateWindow(appData->window), 1);

	appData->vkSwapchain = (VkSwapchainData*) calloc(1, sizeof(VkSwapchainData));
	ExitAssert(appData->vkSwapchain != NULL, 1);
	appData->vkSwapchain->vk     = appData->vk;
	appData->vkSwapchain->window = appData->window;
	ExitAssert(VkSetupSwapchain(appData->vkSwapchain), 1);

	appData->accStructCount = 2;
	appData->accStructs     = (VkAccStruct*) calloc(2, sizeof(VkAccStruct));
	ExitAssert(appData->accStructs != NULL, 1);
	appData->accStructs[0].vk = appData->vk;
	appData->accStructs[1].vk = appData->vk;
	ExitAssert(CreateAS(appData->accStructs + 0, appData->accStructs + 1), 1);

	appData->shaderCount = 2;
	appData->shaders     = (VkShaderData*) calloc(2, sizeof(VkShaderData));
	ExitAssert(appData->shaders != NULL, 1);
	appData->shaders[0].vk = appData->vk;
	appData->shaders[1].vk = appData->vk;

	appData->rtPipeline = (VkRayTracingPipelineData*) calloc(1, sizeof(VkRayTracingPipelineData));
	ExitAssert(appData->rtPipeline != NULL, 1);
	appData->rtPipeline->vk = appData->vk;
	ExitAssert(VkSetupRayTracingPipeline(appData->rtPipeline), 1);

	WLRTMakeWindowVisible(appData->window);

	double lastFrameTime = glfwGetTime();
	double timer         = 0.0;
	while (!appData->window->wantsClose)
	{
		double frameTime = glfwGetTime();
		double deltaTime = frameTime - lastFrameTime;
		lastFrameTime    = frameTime;
		if ((timer += deltaTime) > 0.5)
		{
			printf("%10.2f\n", 1.0 / deltaTime);
			timer = 0.0;
		}

		WLRTWindowPollEvents();

		VkSwapchainData* swapchains[] = { appData->vkSwapchain };
		ExitAssert(VkBeginFrame(appData->vk, swapchains, sizeof(swapchains) / sizeof(*swapchains)), 2);

		VkFrameData* frame = VkGetCurrentFrame(appData->vk);

		for (uint32_t i = 0; i < sizeof(swapchains) / sizeof(*swapchains); ++i)
		{
			VkSwapchainData* swapchain = swapchains[i];

			VkRenderingAttachmentInfo colorAttachment = {
				.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.pNext              = NULL,
				.imageView          = swapchain->views[swapchain->imageIndex],
				.imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.resolveMode        = VK_RESOLVE_MODE_NONE,
				.resolveImageView   = NULL,
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue.color   = {186.0f / 255.0f, 218.0f / 255.0f, 85.0f / 255.0f, 1.0f}
			};
			VkRenderingInfo renderingInfo = {
				.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.pNext                = NULL,
				.flags                = 0,
				.renderArea.offset    = {0, 0},
				.renderArea.extent    = swapchains[i]->extent,
				.layerCount           = 1,
				.viewMask             = 0,
				.colorAttachmentCount = 1,
				.pColorAttachments    = &colorAttachment,
				.pDepthAttachment     = NULL,
				.pStencilAttachment   = NULL
			};
			vkCmdBeginRendering(frame->buffer, &renderingInfo);

			vkCmdEndRendering(frame->buffer);
		}

		ExitAssert(VkEndFrame(appData->vk), 2);
	}

	return 0;
}