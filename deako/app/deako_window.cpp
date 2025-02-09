#include "deako_pch.h"
#include "deako_window.h"

#include "deako_input.h"

namespace Deako {

	static void GLFWErrorCallback(int error, const char* description)
	{
		DK_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window::Window(Context* context, const glm::vec2& size)
		: m_Context(context), m_Size(size)
	{
		DK_ASSERT(context, "Window requires a Context!");

		int success = glfwInit();
		DK_ASSERT(success, "GLFW was not initialized!");
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // set to none for vulkan
		glfwSetErrorCallback(GLFWErrorCallback);

		m_GlfwWindow = glfwCreateWindow((int)m_Size.x, (int)m_Size.y, "Deako", nullptr, nullptr);

		// store dpi scale
		glfwGetWindowContentScale(m_GlfwWindow, &m_DpiScale.x, &m_DpiScale.y);

		InitGlfwEventCallbacks();

		DK_CORE_INFO("Window Size: ({0}, {1})", m_Size.x, m_Size.y);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_GlfwWindow);
		glfwTerminate();
	}

	std::pair<uint32_t, uint32_t> Window::GetScaledSize() const
	{
		uint32_t width = m_Size.x * m_DpiScale.x;
		uint32_t height = m_Size.y * m_DpiScale.y;
		return { width, height };
	}

	void Window::InitGlfwEventCallbacks()
	{
		// Gives easy access to the windows' data in glfw callbacks via a pointer
		glfwSetWindowUserPointer(m_GlfwWindow, this);

		glfwSetWindowSizeCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, int width, int height)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);
				deakoWindow.SetSize((float)width, (float)height);

				WindowResizeEvent e{ width, height };
				deakoWindow.HandleEvent(e);
			});

		glfwSetWindowIconifyCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, int iconified)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);

				if (iconified)
				{
					WindowMinimizedEvent e;
					deakoWindow.HandleEvent(e);
				}
				else
				{
					WindowRestoredEvent e;
					deakoWindow.HandleEvent(e);
				}
			});

		glfwSetWindowCloseCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);
				WindowCloseEvent e;
				deakoWindow.HandleEvent(e);
			});

		//glfwSetFramebufferSizeCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, int width, int height)
		//	{
		//		// TODO: handle when impl vulkan
		//	});

		glfwSetWindowContentScaleCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, float xScale, float yScale)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);
				deakoWindow.SetDpiScale(xScale, yScale);
			});

		glfwSetKeyCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, int keycode, int scancode, int action, int mods)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);

				if (action == GLFW_PRESS)
				{
					KeyPressedEvent e{ static_cast<KeyCode>(keycode), false };
					deakoWindow.HandleEvent(e);
				}
				else if (action == GLFW_RELEASE)
				{
					KeyReleasedEvent e{ static_cast<KeyCode>(keycode) };
					deakoWindow.HandleEvent(e);
				}
				else if (action == GLFW_REPEAT)
				{
					KeyPressedEvent e{ static_cast<KeyCode>(keycode), true };
					deakoWindow.HandleEvent(e);
				}
			});

		glfwSetCharCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, unsigned int keycode)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);

				KeyTypedEvent e{ static_cast<KeyCode>(keycode) };
				deakoWindow.HandleEvent(e);
			});

		glfwSetMouseButtonCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, int button, int action, int mods)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);

				if (action == GLFW_PRESS)
				{
					MouseButtonPressedEvent e{ static_cast<MouseCode>(button) };
					deakoWindow.HandleEvent(e);
				}
				else if (action == GLFW_RELEASE)
				{
					MouseButtonReleasedEvent e{ static_cast<MouseCode>(button) };
					deakoWindow.HandleEvent(e);
				}
			});

		glfwSetScrollCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, double xOffset, double yOffset)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);

				MouseScrolledEvent e{ glm::vec2(xOffset, yOffset) };
				deakoWindow.HandleEvent(e);
			});

		glfwSetCursorPosCallback(m_GlfwWindow, [](GLFWwindow* glfwWindow, double xPos, double yPos)
			{
				Window& deakoWindow = *(Window*)glfwGetWindowUserPointer(glfwWindow);

				MouseMovedEvent e(glm::vec2(xPos, yPos));
				deakoWindow.HandleEvent(e);
			});

	}

}