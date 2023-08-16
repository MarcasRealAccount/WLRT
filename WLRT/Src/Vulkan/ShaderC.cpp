#include "ShaderC.h"
#include "Logging.h"

#include <shaderc/shaderc.hpp>

static shaderc::Compiler s_Compiler;
static WLRTLoggerData    s_Logger;

static struct Init
{
	Init()
	{
		WLRTLoggerSetup(&s_Logger, WLRTStringViewCreate("ShaderC", 7));
	}
} s_Init;

extern "C"
{
	WLRTDynArray WLRTShaderCompile(const WLRTPath* filepath, WLRTStringView source)
	{
		shaderc::CompileOptions options {};
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetTargetSpirv(shaderc_spirv_version_1_6);
		auto         result = s_Compiler.CompileGlslToSpv(source.string, source.length, shaderc_glsl_infer_from_source, filepath->path, options);
		WLRTDynArray out;
		WLRTDynArraySetup(&out, 0, sizeof(uint32_t));
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			auto errorMessage = result.GetErrorMessage();
			WLRTLoggerError(&s_Logger, WLRTStringViewCreate(errorMessage.c_str(), errorMessage.size()));
			return out;
		}
		const uint32_t* begin = result.begin();
		const uint32_t* end   = result.end();
		WLRTDynArrayResize(&out, end - begin);
		memcpy(out.data, begin, end - begin);
		return out;
	}
}