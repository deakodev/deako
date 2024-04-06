#include "Application.h"
#include "dkpch.h"

#include "Input.h"
#include "Deak/Events/ApplicationEvent.h"
#include "Deak/Renderer/Renderer.h"

#include "Deak/Utils/PlatformUtils.h"

namespace Deak
{

    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        DK_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        m_Window = Window::Create();
        m_Window->SetEventCallback(DK_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }

    Application::~Application()
    {
        Renderer::Shutdown();
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
    }

    void Application::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(DK_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(DK_BIND_EVENT_FN(Application::OnWindowResize));
        dispatcher.Dispatch<WindowMinimizedEvent>(DK_BIND_EVENT_FN(Application::OnWindowMinimized));
        dispatcher.Dispatch<WindowRestoredEvent>(DK_BIND_EVENT_FN(Application::OnWindowRestored));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
        {
            (*--it)->OnEvent(event);
            if (event.Handled)
                break;
        }
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = Time::GetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            if (!m_Minimized)
            {
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(timestep);
            }

            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
                layer->OnImGuiRender();
            m_ImGuiLayer->End();

            m_Window->OnUpdate();

            // TODO: Remove later
            if (Input::IsKeyPressed(Deak::Key::Escape))
                Application::Close();
        }
    }

    void Application::Close()
    {
        m_Running = false;
    }


    bool Application::OnWindowClose(WindowCloseEvent& event)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& event)
    {
        Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());

        return false;
    }

    bool Application::OnWindowMinimized(WindowMinimizedEvent& event)
    {
        m_Minimized = true;
        return true;
    }

    bool Application::OnWindowRestored(WindowRestoredEvent& event)
    {
        m_Minimized = false;
        return true;
    }

}
