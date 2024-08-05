#include "MacWindow.h"
#include "dkpch.h"

#include "Deako/Events/WindowEvent.h"
#include "Deako/Events/MouseEvent.h"
#include "Deako/Events/KeyEvent.h"

// #include "System/Vulkan/VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    VkInstance MacWindow::m_VkInstance;

    static uint8_t s_WindowCount = 0;

    static void GLFWErrorCallback(int error, const char* description)
    {
        DK_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    MacWindow::MacWindow(const WindowProps& props)
    {
        Init(props);
    }

    MacWindow::~MacWindow()
    {
        CleanUp();
    }

    void MacWindow::Init(const WindowProps& props)
    {
        m_WindowData.title = props.title;
        m_WindowData.width = props.width;
        m_WindowData.height = props.height;
        m_WindowData.vsync = true;

        DK_CORE_INFO("Creating window - {0} ({1}, {2})", props.title, props.width, props.height);

        if (s_WindowCount == 0)
        {
            int success = glfwInit();
            DK_CORE_ASSERT(success, "GLFW was not initialized!");
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // default is OpenGL, so set to none
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        m_Window = glfwCreateWindow((int)props.width,
            (int)props.height, props.title, nullptr, nullptr);
        ++s_WindowCount;

        // Gives easy access to the windows' data in the callbacks via a pointer
        glfwSetWindowUserPointer(m_Window, &m_WindowData);

        // GLFW Callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                data.width = width;
                data.height = height;

                WindowResizeEvent event(width, height);
                data.EventCallback(event);
            });

        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                if (iconified)
                {
                    WindowMinimizedEvent event;
                    data.EventCallback(event);
                }
                else
                {
                    WindowRestoredEvent event;
                    data.EventCallback(event);
                }
            });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                WindowCloseEvent event;
                data.EventCallback(event);
            });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                switch (action)
                {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(static_cast<KeyCode>(key), 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(static_cast<KeyCode>(key));
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(static_cast<KeyCode>(key), 1);
                    data.EventCallback(event);
                    break;
                }
                }
            });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                KeyTypedEvent event(static_cast<KeyCode>(keycode));
                data.EventCallback(event);
            });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                switch (action)
                {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
                    data.EventCallback(event);
                    break;
                }
                }
            });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                MouseScrolledEvent event((float)xOffset, (float)yOffset);
                data.EventCallback(event);
            });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
            {
                // MacOs specific fix to prevent mouse moved event from firing when cursor is outside of window
                if (glfwGetWindowAttrib(window, GLFW_HOVERED))
                {
                    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                    MouseMovedEvent event((float)xPos, (float)yPos);
                    data.EventCallback(event);
                }
            });

        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
            {
                // VulkanBase::GetState()->framebufferResized = true;

                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                data.width = width;
                data.height = height;
            });
    }

    void MacWindow::CleanUp()
    {
        glfwDestroyWindow(m_Window);
        --s_WindowCount;

        if (s_WindowCount == 0)
        {
            glfwTerminate();
        }
    }

    void MacWindow::OnUpdate()
    {
        glfwPollEvents();
    }

    bool MacWindow::IsVSync() const
    {
        return m_WindowData.vsync;
    }

    void MacWindow::SetVSync(bool enabled)
    {
        // TODO: vulkan
    }

    std::pair<uint32_t, uint32_t> MacWindow::GetWindowFramebufferSize()
    {
        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);
        return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }

}
