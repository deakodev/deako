#include "Input.h"
#include "dkpch.h"


#ifdef DK_PLATFORM_MAC
#include "Platform/MacOS/MacInput.h"
#endif

namespace Deak {

    Scope<Input> Input::s_Instance = Input::Create();

    Scope<Input> Input::Create()
    {
        #ifdef DK_PLATFORM_MAC
        return CreateScope<MacInput>();
        #else
        DK_CORE_ASSERT(false, "Unknown platform!");
        return nullptr;
        #endif
    }

}
