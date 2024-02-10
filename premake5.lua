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

include "Deak/vendor/GLFW"
include "Deak/vendor/Glad"
include "Deak/vendor/imgui"

project "Deak"
    location "Deak"
    kind "SharedLib"
    language "C++"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

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
        "%{IncludeDir.ImGui}"
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
        defines "DK_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "DK_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "DK_DIST"
        optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

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
		"Deak/src"
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
		defines "DK_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "DK_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "DK_DIST"
		optimize "On"
