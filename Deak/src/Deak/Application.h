#pragma once

#include "Core.h"
#include "Core/Window.h"
#include "Core/LayerStack.h"
#include "Deak/Events/Event.h"
#include "Deak/Events/ApplicationEvent.h"

namespace Deak
{
    class DEAK_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running{ true };
        LayerStack m_LayerStack;
    };

    // To be defined in CLIENT (Sandbox.cpp)
    Application* CreateApplication();

}
