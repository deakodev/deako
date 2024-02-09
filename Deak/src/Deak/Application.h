#pragma once

#include "Core.h"
#include "Deak/Events/Event.h"
#include "Deak/Events/ApplicationEvent.h"
#include "Core/Window.h"

namespace Deak
{
    class DEAK_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);

    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running{ true };

    };

    // To be defined in CLIENT (Sandbox.cpp)
    Application* CreateApplication();

}
