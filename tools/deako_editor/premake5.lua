project "deako_editor"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "src",

	  "%{IncludeDir.deako}",
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
      "deako",
      "vulkan-1",
   }