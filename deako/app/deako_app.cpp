#include "deako_pch.h"
#include "deako_app.h"

#include <GLFW/glfw3.h>

namespace Deako {

	Layer::Layer(const char* name)
		: m_DebugName(name)
	{
	}

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();

			delete layer;
			layer = nullptr;
		}
	}

	void LayerStack::OnUpdate()
	{
		for (Layer* layer : m_Layers)
			layer->OnUpdate();
	}

	void LayerStack::OnEvent(Event& e)
	{
		for (Layer* layer : m_Layers)
		{
			if (e.Handled) break;
			layer->OnEvent(e);
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(uint32_t count)
	{
		for (uint32_t i = 0; i < count && m_LayerInsertIndex > 0; ++i)
		{
			if (m_LayerInsertIndex > 0)
			{
				Layer* layer = m_Layers[m_LayerInsertIndex - 1];
				layer->OnDetach();
				m_Layers.erase(m_Layers.begin() + --m_LayerInsertIndex);
			}
		}
	}

	void LayerStack::PopOverlay(uint32_t count)
	{
		for (uint32_t i = 0; i < count && !m_Layers.empty(); ++i)
		{
			Layer* overlay = m_Layers.back();
			overlay->OnDetach();
			m_Layers.pop_back();
		}
	}

	Application::Application(Context* context, const char* name, const glm::vec2& windowSize)
		: m_Context(context), m_Name(name), m_Running(true), m_Active(true)
	{
		DK_CORE_ASSERT(context, "Application requires a Context!");
		DK_CORE_ASSERT(!context->Application, "Context already has a Application!");

		m_Window = CreateScope<Window>(context, windowSize);
		m_Window->SetHandleEventCallback(DK_BIND_EVENT_FN(Application::OnEvent));

		m_Input = CreateScope<Input>(context, m_Window.get());

		DK_CORE_INFO("Created `{}`!", name);
	}

	void Application::Run()
	{
		while (m_Running)
		{
			if (m_Active)
				m_LayerStack.OnUpdate();

			PollEvents();
		}
	}

	void Application::PollEvents()
	{
		glfwPollEvents();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher{ e };
		dispatcher.Dispatch<WindowCloseEvent>(DK_BIND_EVENT_FN(Application::OnClose));
		dispatcher.Dispatch<WindowMinimizedEvent>(DK_BIND_EVENT_FN(Application::OnMinimized));
		dispatcher.Dispatch<WindowRestoredEvent>(DK_BIND_EVENT_FN(Application::OnRestored));

		m_LayerStack.OnEvent(e);
	}

	bool Application::OnClose(WindowCloseEvent& e)
	{
		m_Running = false;
		DK_CORE_INFO("WindowCloseEvent");
		return true;
	}

	bool Application::OnMinimized(WindowMinimizedEvent& e)
	{
		m_Active = false;
		DK_CORE_INFO("WindowMinimizedEvent");
		return true;
	}

	bool Application::OnRestored(WindowRestoredEvent& e)
	{
		m_Active = true;
		DK_CORE_INFO("WindowRestoredEvent");
		return true;
	}

}