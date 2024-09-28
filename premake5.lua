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
    IncludeDir["gli"] = "Deako/vendor/gli"
    IncludeDir["vulkan"] = "Deako/vendor/vulkan/1.3.280.1/macOS/include"
    IncludeDir["vma"] = "Deako/vendor/vma"
    IncludeDir["stb_image"] = "Deako/vendor/stb_image"
    IncludeDir["tinygltf"] = "Deako/vendor/tinygltf"
    IncludeDir["basisu_transcoder"] = "Deako/vendor/basisu/transcoder"
    IncludeDir["basisu_zstd"] = "Deako/vendor/basisu/zstd"
    IncludeDir["imgui"] = "Deako/vendor/imgui"
    IncludeDir["entt"] = "Deako/vendor/entt/include"
    IncludeDir["yaml_cpp"] = "Deako/vendor/yaml-cpp/include"

    LibDir = {}
    LibDir["vulkan"] = "Deako/vendor/vulkan/1.3.280.1/macOS/lib"

    include "Deako/vendor/glfw"
    include "Deako/vendor/imgui"
    include "Deako/vendor/yaml-cpp"

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
        "%{prj.name}/vendor/gli/gli/gli.hpp",
        "%{prj.name}/vendor/gli/gli/**.inl",
        "%{prj.name}/vendor/vma/**.h",
        "%{prj.name}/vendor/vma/**.cpp",
        "%{prj.name}/vendor/tinygltf/**.h",
        "%{prj.name}/vendor/tinygltf/**.cpp",
        "%{prj.name}/vendor/basisu/transcoder/basisu_transcoder.h",
        "%{prj.name}/vendor/basisu/transcoder/basisu_transcoder.cpp",
        "%{prj.name}/vendor/basisu/zstd/zstd.h",
        "%{prj.name}/vendor/basisu/zstd/zstd.c",
        "%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
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
        "%{IncludeDir.gli}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vma}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.basisu_transcoder}",
        "%{IncludeDir.basisu_zstd}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.yaml_cpp}",
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
        "yaml-cpp",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework"
    }

    buildoptions { "-Wno-nullability-completeness" }

    filter "system:macosx"
        systemversion "11.0"

        defines
        {
            "GLFW_INCLUDE_VULKAN",
            "VK_USE_PLATFORM_MACOS_MVK"
        }
    
    filter "files:vendor/imguizmo/**.cpp"
        flags { "NoPCH" }
    
    filter "files:**.c"
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
    prebuildcommands 
    {
        "/bin/zsh ../compile-shaders.sh"
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
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vma}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.basisu_transcoder}",
        "%{IncludeDir.basisu_zstd}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.yaml_cpp}",
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
        "yaml-cpp",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework"
    }

    linkoptions
    {
        "-rpath %{LibDir.vulkan}"
    }

    buildoptions { "-Wno-nullability-completeness" }

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
