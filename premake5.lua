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
IncludeDir["entt"] = "Deak/vendor/entt/include"
IncludeDir["yaml_cpp"] = "Deak/vendor/yaml-cpp/include"
IncludeDir["imguizmo"] = "Deak/vendor/imguizmo"

include "Deak/vendor/GLFW"
include "Deak/vendor/Glad"
include "Deak/vendor/imgui"
include "Deak/vendor/yaml-cpp"

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
		"%{prj.name}/src/**.mm",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/imguizmo/Imguizmo.h",
		"%{prj.name}/vendor/imguizmo/Imguizmo.cpp",
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
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.imguizmo}"
    }

    links 
    { 
        "GLFW",
        "Glad",
        "ImGui",
		"yaml-cpp",
        "OpenGL.framework",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework"
    }

    filter {"system:macosx"}
		systemversion "11.0"

        defines
        {
            "GLFW_INCLUDE_NONE"
        }

	filter "files:vendor/imguizmo/**.cpp"
		flags { "NoPCH" }
	
	filter "files:**.mm"
		flags { "NoPCH" }
		buildoptions 
		{
			"-x objective-c++"
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
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
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


project "Deak-Editor"
location "Deak-Editor"
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
	"%{IncludeDir.glm}",
	"%{IncludeDir.entt}",
	"%{IncludeDir.yaml_cpp}",
	"%{IncludeDir.imguizmo}"
}

links
{
	"Deak",
	"GLFW",
	"Glad",
	"ImGui",
	"yaml-cpp",
	"OpenGL.framework",
	"Cocoa.framework",
	"IOKit.framework",
	"CoreVideo.framework"
}

filter "system:macosx"
	systemversion "11.0"

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
