#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <shaderc/shaderc.hpp>

static shaderc::Compiler s_Compiler;

extern "C"
{
	bool ShaderCCompileShader(const char* filepath, const char* shaderSource, size_t shaderSourceLength, uint32_t** code, size_t* codeSize)
	{
		shaderc::CompileOptions options {};
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetTargetSpirv(shaderc_spirv_version_1_6);
		auto result = s_Compiler.CompileGlslToSpv(shaderSource, shaderSourceLength, shaderc_glsl_infer_from_source, filepath, options);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			auto errorMessage = result.GetErrorMessage();
			std::printf("ShaderC ERROR: %s\n", errorMessage.c_str());
			return false;
		}

		*codeSize = result.end() - result.begin();
		*code     = (uint32_t*) malloc(*codeSize * sizeof(uint32_t));
		memcpy(*code, result.begin(), *codeSize);
		return true;
	}

	void ShaderCFreeBuffer(uint32_t* code, size_t codeSize)
	{
		(void) codeSize;
		free(code);
	}
}