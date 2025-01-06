#include "deako_pch.h"
#include "deako_input.h"

#include "deako_window.h"

#include <GLFW/glfw3.h>

namespace Deako {

	Input::Input(Context* context, Window* window)
		: m_Context(context), m_Window(window)
	{
		DK_CORE_ASSERT(context, "Input required a Context!");
		DK_CORE_ASSERT(window, "Input requires a Window!");
	}

	Input::~Input()
	{
	}

	bool Input::IsKeyPressed(KeyCode key) const
	{
		auto state = glfwGetKey(m_Window->GetGlfwWindow(), (int32_t)key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseClicked(MouseCode button) const
	{
		auto state = glfwGetMouseButton(m_Window->GetGlfwWindow(), (int32_t)button);
		return state == GLFW_PRESS;
	}

}

