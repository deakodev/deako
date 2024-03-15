workspace "Deak_Engine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Deak/vendor/GLFW/include"
IncludeDir["Glad"] = "Deak/vendor/Glad/include"
IncludeDir["ImGui"] = "Deak/vendor/imgui"
IncludeDir["glm"] = "Deak/vendor/glm"

include "Deak/vendor/GLFW"
include "Deak/vendor/Glad"
include "Deak/vendor/imgui"

project "Deak"
    location "Deak"
    kind "SharedLib"
    language "C++"
	staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	pchheader "dkpch.h"
	pchsource "Deak/src/dkpch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
    }

    links 
    { 
        "GLFW",
        "Glad",
        "ImGui",
        "OpenGL.framework",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework"
    }

    filter "system:macosx"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "11.0"

        defines
        {
            "DK_PLATFORM_MAC",
            "GLFW_INCLUDE_NONE"
        }

		postbuildcommands
		{
			("cp %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

    filter "configurations:Debug"
		symbols "On"
		defines "DK_DEBUG"
		runtime "Debug"

    filter "configurations:Release"
        defines "DK_RELEASE"
        optimize "On"
		runtime "Release"

    filter "configurations:Dist"
        defines "DK_DIST"
        optimize "On"
		runtime "Release"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Deak/vendor/spdlog/include",
		"Deak/src",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Deak"
	}

	filter "system:macosx"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "11.0"

		defines
		{
			"DK_PLATFORM_MAC"
		}

	filter "configurations:Debug"
		symbols "On"
        defines "DK_DEBUG"
		runtime "Debug"

	filter "configurations:Release"
		defines "DK_RELEASE"
		optimize "On"
		runtime "Release"

	filter "configurations:Dist"
		defines "DK_DIST"
		optimize "On"
		runtime "Release"
