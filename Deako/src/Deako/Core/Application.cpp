#include "Application.h"
#include "dkpch.h"

namespace Deako
{
    static Application* s_ApplicationInstance = nullptr;

    Application& InitApplication(Application* application)
    {
        DK_CORE_ASSERT(!s_ApplicationInstance, "Application already exists!");
        s_ApplicationInstance = application;

        return *s_ApplicationInstance;
    }

    void DestroyApplication()
    {
        delete s_ApplicationInstance;
        s_ApplicationInstance = nullptr;
    }

    Application& GetApplication()
    {
        DK_CORE_ASSERT(s_ApplicationInstance, "Application instance not initialized!");
        return *s_ApplicationInstance;
    }

    Application::Application(const ApplicationSpecification& specification)
        : m_Specification(specification)
    {
        m_Window = Window::Create(WindowProps(specification.name));
        m_Window->SetEventCallback(DK_BIND_EVENT_FN(Application::OnEvent));
    }

    void Application::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(DK_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowMinimizedEvent>(DK_BIND_EVENT_FN(Application::OnWindowMinimized));
        dispatcher.Dispatch<WindowRestoredEvent>(DK_BIND_EVENT_FN(Application::OnWindowRestored));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (event.Handled)
                break;
            (*it)->OnEvent(event);
        }
    }

    void Application::Run()
    {
        while (m_Running)
        {
            if (!m_Minimized)
            {
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate();

                m_Window->OnUpdate();
            }
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& event)
    {
        m_Running = false;
        return true;
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
