#pragma once

#include "Core.h"
#include "Deak/Events/Event.h"

namespace Deak
{
    class DEAK_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();
    };

    // To be defined in CLIENT (Sandbox.cpp)
    Application* CreateApplication();

}
