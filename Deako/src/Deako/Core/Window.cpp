#include "Window.h"
#include "dkpch.h"

#ifdef DK_PLATFORM_MAC
#include "System/MacOS/MacWindow.h"
#endif

namespace Deako {

    Scope<Window> Window::Create(const WindowProps& props)
    {
        #ifdef DK_PLATFORM_MAC
        return CreateScope<MacWindow>(props);
        #else
        DK_CORE_ASSERT(false, "Unknown platform!");
        return nullptr;
        #endif   
    }

}
