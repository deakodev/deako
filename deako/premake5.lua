project "deako"
   kind "StaticLib"
   language "C"
   cdialect "C99"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   pchheader "deako_pch.h"
   pchsource "deako_pch.c"

   files { "**.h", "**.c" }

   includedirs
   {
      "%{prj.location}", 
      "%{IncludeDir.cglm}",
      "%{IncludeDir.log}",
      "%{IncludeDir.glfw}", 
      "%{IncludeDir.magic_memory}", 
      "%{IncludeDir.vulkan}", 
   }

   libdirs
   {
       "%{LibDir.vulkan}",
   }

   links
   {
       "log",
       "glfw",
       "magic_memory",
       "vulkan-1",
   }

   filter "system:windows"
      systemversion "latest"
      defines
      {
         "GLFW_INCLUDE_VULKAN",
      }
