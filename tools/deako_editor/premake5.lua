project "deako_editor"
   kind "ConsoleApp"
   language "C"
   cdialect "C99"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   files { "**.h", "**.c" }

   includedirs
   {
      "src",
	  "%{IncludeDir.deako}",
      "%{IncludeDir.glm}",
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
      "deako",
      "vulkan-1",
   }

   filter "system:windows"
      systemversion "latest"
      defines
      {
         "GLFW_INCLUDE_VULKAN",
      }