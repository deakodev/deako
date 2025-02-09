#pragma once

#include "deako_event.h"

#include <Windows.h> // Ensure this is included before glfw
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Deako {

	using EventCallbackFn = std::function<void(Event&)>;

	class Window
	{
	public:
		Window(Context* context, const glm::vec2& size = glm::vec2{ 1600, 900 });
		~Window();

		GLFWwindow* GetGlfwWindow() const { return m_GlfwWindow; }
		std::pair<uint32_t, uint32_t> GetScaledSize() const;

		void SetSize(float width, float height) { m_Size = { width, height }; }
		void SetDpiScale(float xScale, float yScale) { m_DpiScale = { xScale, yScale }; }
		void SetHandleEventCallback(const EventCallbackFn& callback) { HandleEvent = callback; }

		void InitGlfwEventCallbacks();

		EventCallbackFn HandleEvent = [](Event&) {};

	private:
		Context* m_Context;
		glm::vec2 m_Size;
		glm::vec2 m_DpiScale;

		GLFWwindow* m_GlfwWindow; // owned by window
	};

}


