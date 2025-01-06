#pragma once

#include "deako_event.h"
#include "deako_window.h"

namespace Deako {

	class Layer
	{
	public:
		Layer(const char* name = "Deako Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& e) {}

		const char* GetName() const { return m_DebugName; }

	protected:
		const char* m_DebugName;
	};

	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		void OnUpdate();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(uint32_t count = 1);
		void PopOverlay(uint32_t count = 1);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<Layer*>::const_iterator end()	const { return m_Layers.end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }

	private:
		std::vector<Layer*> m_Layers;
		uint32_t m_LayerInsertIndex = 0;
	};

	class Application
	{
	public:
		Application(Context* context, const char* name, const glm::vec2& windowSize);

		void Run();
		void PollEvents();
		void Shutdown() { m_Running = false; }

		template<typename T>
		void CreateLayer(const char* name)
		{
			T* layer = new T(name);
			m_LayerStack.PushLayer(layer);
		}

		void OnEvent(Event& e);
		bool OnClose(WindowCloseEvent& e);
		bool OnMinimized(WindowMinimizedEvent& e);
		bool OnRestored(WindowRestoredEvent& e);

		Window* GetWindow() { return m_Window.get(); }

	protected:
		LayerStack m_LayerStack;

	private:
		Context* m_Context; // parent, not owned
		Scope<Window> m_Window;
		Scope<Input> m_Input;

		const char* m_Name;
		bool m_Running;
		bool m_Active;
	};

}
