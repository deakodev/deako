#pragma once

#include "Core.h"
#include "Core/Window.h"
#include "Core/LayerStack.h"
#include "Deak/Events/Event.h"
#include "Deak/Events/ApplicationEvent.h"
#include "Deak/ImGui/ImGuiLayer.h"

#include "Deak/Renderer/Shader.h"
#include "Deak/Renderer/Buffer.h"

namespace Deak
{
    class Application
    {
    public:
        Application();
        virtual ~Application() = default;

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

        unsigned int m_VertexArray;
        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<VertexBuffer> m_VertexBuffer;
        std::unique_ptr<IndexBuffer> m_IndexBuffer;

    private:
        static Application* s_Instance;
    };

    // To be defined in CLIENT (Sandbox.cpp)
    Application* CreateApplication();

}
