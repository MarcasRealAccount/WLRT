workspace("WLRT")
	common:addConfigs()
	common:addBuildDefines()

	cdialect("C17")
	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("Off")
	flags({
		"MultiProcessorCompile",
		"FatalCompileWarnings"
	})
	conformancemode("On")

	startproject("WLRT")
	project("WLRT")
		location("WLRT/")
		warnings("Extra")
		kind("ConsoleApp")

		common:outDirs()
		common:debugDir()

		includedirs({ "%{prj.location}/Src/" })
		files({ "%{prj.location}/Src/**" })
		removefiles({ "*.DS_Store" })

		pkgdeps({ "stb", "glfw", "vulkan-sdk:shaderc=true" })

		common:addActions()