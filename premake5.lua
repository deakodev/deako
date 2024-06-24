workspace "Deako"
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

project "Deako"
    location "Deako"
    kind "StaticLib"
    language "C++"
	cppdialect "C++20"
	staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "dkpch.h"
	pchsource "Deako/src/dkpch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.mm"
    }

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

    includedirs
    {
        "%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include"
    }

    links 
    { 
    }

    filter "system:macosx"
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


project "Deako-Editor"
location "Deako-Editor"
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
	"Deako/src",
	"Deako/vendor",
	"Deako/vendor/spdlog/include"
}

links
{
	"Deako"
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
