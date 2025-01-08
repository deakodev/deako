project "deako"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   pchheader "deako_pch.h"
   pchsource "deako_pch.cpp"

   files { "**.h", "**.cpp" }

   includedirs
   {
      "%{prj.location}", 
     
      "%{IncludeDir.spdlog}",
      "%{IncludeDir.glm}",
      "%{IncludeDir.glfw}", 
      "%{IncludeDir.vulkan}", 
   }

   libdirs
   {
       "%{LibDir.vulkan}",
   }

   links
   {
       "glfw",
       "vulkan-1",
   }

   filter "system:windows"
      systemversion "latest"
      defines
      {
         "GLFW_INCLUDE_VULKAN",
      }
