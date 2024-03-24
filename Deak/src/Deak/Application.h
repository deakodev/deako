#pragma once

#include "Core.h"
#include "Core/Window.h"
#include "Core/LayerStack.h"
#include "Deak/Events/Event.h"
#include "Deak/Events/ApplicationEvent.h"
#include "Deak/ImGui/ImGuiLayer.h"

#include "Deak/Renderer/Shader.h"

namespace Deak
{
    class Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        inline static Application& Get() { return *s_Instance; }
        inline Window& GetWindow() { return *m_Window; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running{ true };
        LayerStack m_LayerStack;

        unsigned int m_VertexArray, m_VertexBuffer, m_IndexBuffer;
        std::unique_ptr<Shader> m_Shader; // Temporary shader for triangle

    private:
        static Application* s_Instance;
    };

    // To be defined in CLIENT (Sandbox.cpp)
    Application* CreateApplication();

}
