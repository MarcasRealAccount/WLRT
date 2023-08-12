#include "Filesystem.h"
#include "FileWatcher.h"
#include "Vk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool ShaderCCompileShader(const char* filepath, const char* shaderSource, size_t shaderSouceLength, uint32_t** code, size_t* codeSize);
void ShaderCFreeBuffer(uint32_t* code, size_t codeSize);

static void VkReadShaderCache(VkShaderData* shader, uint32_t** code, size_t* codeSize)
{
	FSPath cachePath = FSCreatePath("Cache", ~0ULL);
	if (!FSPathAppend(&cachePath, &shader->filepath))
	{
		*code     = NULL;
		*codeSize = 0;
		FSDestroyPath(&cachePath);
		return;
	}

	uint64_t cacheWt = FSLastWriteTime(&cachePath);
	uint64_t codeWt  = FSLastWriteTime(&shader->filepath);
	if (codeWt > cacheWt)
	{
		*code     = NULL;
		*codeSize = 0;
		FSDestroyPath(&cachePath);
		return;
	}

	FILE* cacheFile = fopen(cachePath.buf, "rb");
	if (!cacheFile)
	{
		*code     = NULL;
		*codeSize = 0;
		FSDestroyPath(&cachePath);
		return;
	}
	fseek(cacheFile, 0, SEEK_END);
	*codeSize = ftell(cacheFile);
	fseek(cacheFile, 0, SEEK_CUR);
	*code = (uint32_t*) malloc(*codeSize);
	if (!*code)
	{
		*code     = NULL;
		*codeSize = 0;
		fclose(cacheFile);
		FSDestroyPath(&cachePath);
		return;
	}
	fread(*code, 1, *codeSize, cacheFile);
	fclose(cacheFile);
	FSDestroyPath(&cachePath);
}

static bool VkWriteShaderCache(VkShaderData* shader, const uint32_t* code, size_t codeSize, uint64_t codeWt)
{
	FSPath cachePath = FSCreatePath("Cache", ~0ULL);
	if (!FSPathAppend(&cachePath, &shader->filepath))
	{
		FSDestroyPath(&cachePath);
		return false;
	}

	FILE* cacheFile = fopen(cachePath.buf, "wb");
	if (!cacheFile)
	{
		FSDestroyPath(&cachePath);
		return false;
	}
	fwrite(code, 1, codeSize, cacheFile);
	fclose(cacheFile);
	FSSetLastWriteTime(&cachePath, codeWt);
	FSDestroyPath(&cachePath);
	return true;
}

static bool VkCompileShaderCode(VkShaderData* shader, uint32_t** code, size_t* codeSize, uint64_t* codeWt)
{
	*codeWt        = FSLastWriteTime(&shader->filepath);
	FILE* codeFile = fopen(shader->filepath.buf, "r");
	if (!codeFile)
		return false;
	fseek(codeFile, 0, SEEK_END);
	*codeSize = ftell(codeFile);
	fseek(codeFile, 0, SEEK_SET);
	*code = (uint32_t*) malloc(*codeSize);
	if (!*code)
	{
		fclose(codeFile);
		return false;
	}
	fread(*code, 1, *codeSize, codeFile);
	fclose(codeFile);
	return true;
}

static bool VkGetShaderCode(VkShaderData* shader, uint32_t** code, size_t* codeSize)
{
	if (!shader->handle)
		VkReadShaderCache(shader, code, codeSize);

	if (!*code)
	{
		uint64_t wt = 0;
		if (!VkCompileShaderCode(shader, code, codeSize, &wt) ||
			!VkWriteShaderCache(shader, *code, *codeSize, wt))
			return false;
	}

	return true;
}

static void VkShaderModified(const FSPath* filepath, void* userData)
{
	(void) filepath;
	VkShaderData* shader = (VkShaderData*) userData;
	shader->modified     = true;
}

bool VkSetupShader(VkShaderData* shader, const char* filepath)
{
	if (!shader || !shader->vk) return false;
	VkData* vk = shader->vk;

	size_t    codeSize = 0;
	uint32_t* code     = NULL;
	shader->filepath   = FSCreatePath(filepath, ~0ULL);
	shader->modified   = false;
	shader->watchID    = 0;
	if (!VkGetShaderCode(shader, &code, &codeSize))
	{
		VkCleanupShader(shader);
		return false;
	}

	VkShaderModuleCreateInfo createInfo = {
		.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext    = NULL,
		.flags    = 0,
		.codeSize = codeSize,
		.pCode    = code
	};
	if (!VkValidate(vk, vkCreateShaderModule(vk->device, &createInfo, vk->allocation, &shader->handle)))
	{
		VkCleanupShader(shader);
		return false;
	}
	shader->watchID = FWWatchFile(&shader->filepath, &VkShaderModified, shader);
	free(code);
	return true;
}

void VkCleanupShader(VkShaderData* shader)
{
	if (!shader || !shader->vk) return;
	VkData* vk = shader->vk;
	vkDestroyShaderModule(vk->device, shader->handle, vk->allocation);
	shader->handle = NULL;
	FSDestroyPath(&shader->filepath);
	FWUnwatchFile(shader->watchID);
	shader->watchID = 0;
}

bool VkShaderRecompile(VkShaderData* shader)
{
	if (!shader || !shader->vk) return false;
	VkData* vk = shader->vk;

	size_t    codeSize = 0;
	uint32_t* code     = NULL;
	shader->modified   = false;
	if (!VkGetShaderCode(shader, &code, &codeSize))
		return false;

	VkShaderModule           newShaderModule = NULL;
	VkShaderModuleCreateInfo createInfo      = {
			 .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			 .pNext    = NULL,
			 .flags    = 0,
			 .codeSize = codeSize,
			 .pCode    = code
	};
	if (!VkValidate(vk, vkCreateShaderModule(vk->device, &createInfo, vk->allocation, &newShaderModule)))
	{
		free(code);
		return false;
	}
	free(code);
	vkDestroyShaderModule(vk->device, shader->handle, vk->allocation);
	shader->handle = newShaderModule;
	return true;
}