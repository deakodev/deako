#pragma once

#include "Deako/Core/Base.h"
#include "Deako/Core/Window.h"
#include "Deako/Core/LayerStack.h"

#include "Deako/Event/Event.h"
#include "Deako/Event/WindowEvent.h"
#include "Deako/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Deako {

    class Application
    {
    public:
        Application(const char* name = "Deako App");
        virtual ~Application();

        void OnEvent(Event& event);
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        static Application& Get() { return *s_Instance; }
        Window& GetWindow() { return *m_Window; }
        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
        LayerStack& GetLayerStack() { return m_LayerStack; }

    private:
        friend int ::main(int argc, char** argv);
        void Run();

        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowResize(WindowResizeEvent& event);
        bool OnWindowMinimized(WindowMinimizedEvent& event);
        bool OnWindowRestored(WindowRestoredEvent& event);

    private:
        const char* m_AppName;
        static Application* s_Instance;

        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        LayerStack m_LayerStack;

        bool m_Running = true;
        bool m_Minimized = false;
    };

    // To be defined client side
    Application* CreateApplication();

}
