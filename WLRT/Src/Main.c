#include "Vk.h"
#include "VkFuncs/VkFuncs.h"
#include "Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>

static void GLFWErrCB(int code, const char* msg)
{
	printf("GLFW ERROR (%d): %s\n", code, msg);
}

static void VKErrCB(int code, const char* msg)
{
	printf("VK ERROR (%d %s): %s\n", code, VkGetErrorString(code), msg);
}

static bool CreateBLAS(VkData* vk, VkAccStructBuilder* builder, VkAccStruct* blas)
{
	typedef struct Vertex
	{
		float x, y, z, w;
	} Vertex;

	typedef uint32_t Index;

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
	if (!VkAccStructBuilderSetTriangles(vk, builder, 0, vkGetBufferDeviceAddress(vk->device, &vbAddressInfo), VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vertex), sizeof(vertices) / sizeof(*vertices), vkGetBufferDeviceAddress(vk->device, &ibAddressInfo), VK_INDEX_TYPE_UINT32, sizeof(indices) / sizeof(*indices)) ||
		!VkAccStructBuilderPrepare(vk, builder, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR, 1, 0) ||
		!VkAccStructBuilderBuild(vk, builder, &uncompressed) ||
		!VkAccStructBuilderCompact(vk, builder, &uncompressed, blas))
	{
		VkCleanupAccStruct(vk, &uncompressed);
		vmaDestroyBuffer(vk->allocator, vertexBuffer, vertexBufferA);
		vmaDestroyBuffer(vk->allocator, indexBuffer, indexBufferA);
		return false;
	}

	VkCleanupAccStruct(vk, &uncompressed);
	vmaDestroyBuffer(vk->allocator, vertexBuffer, vertexBufferA);
	vmaDestroyBuffer(vk->allocator, indexBuffer, indexBufferA);
	return true;
}

static bool CreateTLAS(VkData* vk, VkAccStructBuilder* builder, VkAccStruct* blas, VkAccStruct* tlas)
{
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
	VkWriteTLASInstance(vk, instancesData, blas, 0, &identityMatrix, 0, 0xFF, 0, 0);
	vmaUnmapMemory(vk->allocator, instancesBufferA);

	VkBufferDeviceAddressInfo iAddressInfo = {
		.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext  = NULL,
		.buffer = instancesBuffer
	};
	if (!VkAccStructBuilderSetInstances(vk, builder, 0, vkGetBufferDeviceAddress(vk->device, &iAddressInfo), 1) ||
		!VkAccStructBuilderPrepare(vk, builder, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, 1, 0) ||
		!VkAccStructBuilderBuild(vk, builder, tlas))
	{
		vmaDestroyBuffer(vk->allocator, instancesBuffer, instancesBufferA);
		return false;
	}

	vmaDestroyBuffer(vk->allocator, instancesBuffer, instancesBufferA);
	return true;
}

static bool CreateAS(VkData* vk, VkAccStruct* blas, VkAccStruct* tlas)
{
	VkAccStructBuilder builder;
	memset(&builder, 0, sizeof(builder));
	if (!VkSetupAccStructBuilder(vk, &builder)) return false;

	if (!CreateBLAS(vk, &builder, blas))
	{
		VkCleanupAccStructBuilder(vk, &builder);
		return false;
	}

	if (!CreateTLAS(vk, &builder, blas, tlas))
	{
		VkCleanupAccStruct(vk, blas);
		VkCleanupAccStructBuilder(vk, &builder);
		return false;
	}

	VkCleanupAccStructBuilder(vk, &builder);
	return true;
}

int main(int argc, char** argv)
{
	glfwSetErrorCallback(&GLFWErrCB);
	if (!glfwInit())
		return 1;

	VkData vk;
	memset(&vk, 0, sizeof(vk));
	vk.framesInFlight = 2;
	vk.errorCallback  = &VKErrCB;
	if (!VkSetup(&vk))
	{
		glfwTerminate();
		return 1;
	}
	VkLoadFuncs(vk.instance, vk.device);

	WindowData window = {
		.handle = NULL,
		.x      = 1 << 31,
		.y      = 1 << 31,
		.width  = 1280,
		.height = 720
	};
	VkSwapchainData vkSwapchain;
	memset(&vkSwapchain, 0, sizeof(vkSwapchain));
	vkSwapchain.window = &window;
	if (!WLRTCreateWindow(&window))
	{
		VkCleanup(&vk);
		glfwTerminate();
		return 1;
	}
	if (!VkSetupSwapchain(&vk, &vkSwapchain))
	{
		WLRTDestroyWindow(&window);
		VkCleanup(&vk);
		glfwTerminate();
		return 1;
	}

	VkAccStruct blas;
	VkAccStruct tlas;
	memset(&blas, 0, sizeof(blas));
	memset(&tlas, 0, sizeof(tlas));
	if (!CreateAS(&vk, &blas, &tlas))
	{
		VkCleanupSwapchain(&vk, &vkSwapchain);
		WLRTDestroyWindow(&window);
		VkCleanup(&vk);
		glfwTerminate();
		return 1;
	}

	WLRTMakeWindowVisible(&window);

	double lastFrameTime = glfwGetTime();
	double timer         = 0.0;
	while (!window.wantsClose)
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

		VkSwapchainData* swapchains[] = { &vkSwapchain };
		if (!VkBeginFrame(&vk, swapchains, sizeof(swapchains) / sizeof(*swapchains))) break;

		VkFrameData* frame = VkGetCurrentFrame(&vk);

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

		if (!VkEndFrame(&vk)) break;
	}

	vkDeviceWaitIdle(vk.device);

	VkCleanupAccStruct(&vk, &blas);
	VkCleanupAccStruct(&vk, &tlas);

	VkCleanupSwapchain(&vk, &vkSwapchain);
	WLRTDestroyWindow(&window);

	VkCleanup(&vk);

	glfwTerminate();
	return 0;
}