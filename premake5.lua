workspace "Deak_Engine"
	architecture "ARM64"

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
IncludeDir["stb_image"] = "Deak/vendor/stb_image"
IncludeDir["glm"] = "Deak/vendor/glm"

include "Deak/vendor/GLFW"
include "Deak/vendor/Glad"
include "Deak/vendor/imgui"

project "Deak"
    location "Deak"
    kind "StaticLib"
    language "C++"
	cppdialect "C++20"
	staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	pchheader "dkpch.h"
	pchsource "Deak/src/dkpch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/glm/glm/**.hpp",
		"%{prj.name}/glm/glm/**.inl"

    }

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
		"%{IncludeDir.stb_image}",
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
        systemversion "11.0"

        defines
        {
            "DK_PLATFORM_MAC",
            "GLFW_INCLUDE_NONE"
        }

    filter "configurations:Debug"
		defines "DK_DEBUG"
		runtime "Debug"
		symbols "on"

    filter "configurations:Release"
        defines "DK_RELEASE"
		runtime "Release"
		optimize "on"

    filter "configurations:Dist"
        defines "DK_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

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
		"Deak/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Deak",
		"GLFW",
        "Glad",
        "ImGui",
        "OpenGL.framework",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework"
	}

	filter "system:macosx"
		systemversion "11.0"

		defines
		{
			"DK_PLATFORM_MAC"
		}

	filter "configurations:Debug"
        defines "DK_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "DK_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "DK_DIST"
		runtime "Release"
		optimize "on"
