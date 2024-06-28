#include "Application.h"
#include "dkpch.h"

#include "Deako/Events/WindowEvent.h"
#include "Deako/Renderer/Renderer.h"

namespace Deako
{

    Application* Application::s_Instance = nullptr;

    Application::Application(const char* name)
    {
        DK_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        m_Window = Window::Create(WindowProps(name));
        m_Window->SetEventCallback(DK_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();
    }

    Application::~Application()
    {
        Renderer::CleanUp();
    }

    void Application::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<WindowCloseEvent>(DK_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(DK_BIND_EVENT_FN(Application::OnWindowResize));
    }

    void Application::Run()
    {
        while (m_Running)
        {
            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& event)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& event)
    {
        // Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());

        return false;
    }

}
