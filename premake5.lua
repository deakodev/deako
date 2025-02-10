workspace "deako"
    architecture "x64"
    startproject "tools/deako_editor"
    configurations { "debug", "release", "dist" }
    flags { "MultiProcessorCompile" }

    filter "system:windows"
        systemversion "latest"
        defines { "DK_PLATFORM_WINDOWS" }

    filter "configurations:debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"

    OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

    IncludeDir = {}
    IncludeDir["deako"] = "%{wks.location}/deako"
	IncludeDir["glm"] = "%{wks.location}/vendors/glm"
	IncludeDir["glfw"] = "%{wks.location}/vendors/glfw/include"
    IncludeDir["magic_memory"] = "%{wks.location}/vendors/magic_memory/magic_memory"
    IncludeDir["vulkan"] = "%{wks.location}/vendors/vulkan/1.3.296.0/Include"

    LibDir = {}
    LibDir["vulkan"] = "%{wks.location}/vendors/vulkan/1.3.296.0/Lib"

    group "vendors"
	    include "vendors/glfw/premake5.lua"
        include "vendors/magic_memory/magic_memory/premake5.lua"
    group ""

    group "deako"
	    include "deako/premake5.lua"
    group ""

    group "tools"
	    include "tools/deako_editor/premake5.lua"
    group ""