project "sandbox"
   kind "ConsoleApp"
   language "C"
   cdialect "C99"
   staticruntime "On"

   targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin/int/" .. OutputDir .. "/%{prj.name}")

   files { "**.h", "**.c" }

   includedirs
   {
      "%{prj.location}", 
      "%{IncludeDir.glfw}", 
   }

   links
   {
   }

   filter { "language:C" }
        warnings "Extra"         -- Enables most warnings

   filter { "toolset:gcc or clang" }
        buildoptions 
        {
            "-Wall",         -- Enable all common warnings
            "-Wextra",       -- Enable extra warnings
            "-pedantic",     -- Enforce strict C standard compliance
            "-Werror"        -- Treat warnings as errors (optional)
        }

   filter { "toolset:msc" }
        buildoptions 
        {
            "/W4",          -- Enable high warning level
            "/WX"           -- Treat warnings as errors (optional)
        }