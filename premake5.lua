workspace "Deako"
	architecture "ARM64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    IncludeDir = {}
    IncludeDir["glm"] = "Deako/vendor/glm"
    IncludeDir["glfw"] = "Deako/vendor/glfw/include"
    IncludeDir["vulkan"] = "Deako/vendor/vulkan/1.3.280.1/macOS/include"
    IncludeDir["imgui"] = "Deako/vendor/imgui"
    IncludeDir["stb_image"] = "Deako/vendor/stb_image"
    IncludeDir["tiny_obj_loader"] = "Deako/vendor/tiny_obj_loader"
    IncludeDir["entt"] = "Deako/vendor/entt/include"

    LibDir = {}
    LibDir["vulkan"] = "Deako/vendor/vulkan/1.3.280.1/macOS/lib"

    include "Deako/vendor/glfw"
    include "Deako/vendor/imgui"

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
        "%{prj.name}/src/**.mm",
        "%{prj.name}/glm/glm/**.hpp",
        "%{prj.name}/glm/glm/**.inl",
        "%{prj.name}/vendor/tiny_obj_loader/**.h",
        "%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.glm}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.tiny_obj_loader}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}"
    }

    libdirs
    {
        "%{LibDir.vulkan}"
    }

    links 
    { 
        "glfw",
        "vulkan",
        "imgui",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework"
    }

    filter "system:macosx"
        systemversion "11.0"

        defines
        {
            "GLFW_INCLUDE_VULKAN",
            "VK_USE_PLATFORM_MACOS_MVK"
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
        defines 
        {
            "DK_DEBUG",
            "VK_VALIDATION"
        }
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

     -- Shader compilation commands
     prebuildcommands {
        "/Users/deakzach/Desktop/Deako/Deako/vendor/vulkan/1.3.280.1/macOS/bin/glslc /Users/deakzach/Desktop/Deako/Deako-Editor/assets/shaders/shader.vert -o /Users/deakzach/Desktop/Deako/Deako-Editor/assets/shaders/bin/shader.vert.spv",
        "/Users/deakzach/Desktop/Deako/Deako/vendor/vulkan/1.3.280.1/macOS/bin/glslc /Users/deakzach/Desktop/Deako/Deako-Editor/assets/shaders/shader.frag -o /Users/deakzach/Desktop/Deako/Deako-Editor/assets/shaders/bin/shader.frag.spv"
        }

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Deako/src",
        "Deako/vendor",
        "Deako/vendor/spdlog/include",
        "%{IncludeDir.glm}",
        "%{IncludeDir.vulkan}" ,
        "%{IncludeDir.entt}"
    }

    libdirs
    {
        "%{LibDir.vulkan}"
    }

    links
    {
        "Deako",
        "glfw",
        "vulkan",
        "imgui",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework"
    }

    linkoptions
        {
            "-rpath %{LibDir.vulkan}"
        }

    filter "system:macosx"
        systemversion "11.0"

        defines
        {
            "GLFW_INCLUDE_VULKAN",
            "VK_USE_PLATFORM_MACOS_MVK"
        }

    filter "configurations:Debug"
        defines 
        {
            "DK_DEBUG",
            "VK_VALIDATION"
        }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines 
        {
            "DK_RELEASE",
            "VK_VALIDATION"
        }
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "DK_DIST"
        runtime "Release"
        optimize "on"
