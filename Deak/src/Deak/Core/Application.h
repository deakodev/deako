#pragma once

#include "Base.h"
#include "Window.h"
#include "LayerStack.h"
#include "Timestep.h"

#include "Deak/Events/Event.h"
#include "Deak/Events/ApplicationEvent.h"
#include "Deak/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Deak
{
    class Application
    {
    public:
        Application(const char* name = "Deak App");
        virtual ~Application();

        void Close();
        void OnEvent(Event& event);
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        static Application& Get() { return *s_Instance; }
        Window& GetWindow() { return *m_Window; }

    private:
        friend int ::main(int argc, char** argv);
        void Run();
        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowResize(WindowResizeEvent& event);
        bool OnWindowMinimized(WindowMinimizedEvent& event);
        bool OnWindowRestored(WindowRestoredEvent& event);

    private:
        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

        static Application* s_Instance;
    };

    // To be defined in CLIENT (Sandbox.cpp)
    Application* CreateApplication();

}
