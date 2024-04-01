#include "dkpch.h"

#include "Deak/Utils/PlatformUtils.h"

#include <GLFW/glfw3.h>

namespace Deak {

    float Time::GetTime()
    {
        return glfwGetTime();
    }

}
