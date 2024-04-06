#include "MacWindow.h"
#include "dkpch.h"

#include "Deak/Events/ApplicationEvent.h"
#include "Deak/Events/MouseEvent.h"
#include "Deak/Events/KeyEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Deak {

    static uint8_t s_GLFWWindowCount = 0;

    static void GLFWErrorCallback(int error, const char* description)
    {
        DK_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Scope<Window> Window::Create(const WindowProps& props)
    {
        return CreateScope<MacWindow>(props);
    }

    MacWindow::MacWindow(const WindowProps& props)
    {
        Init(props);
    }

    MacWindow::~MacWindow()
    {
        Shutdown();
    }

    void MacWindow::Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        DK_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (s_GLFWWindowCount == 0)
        {
            int success = glfwInit();
            DK_CORE_ASSERT(success, "Could not intialize GLFW!");

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            // Have to specify these on macOS
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            // to prevent 1200x800 from becoming 2400x1600
            glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);

            glfwSetErrorCallback(GLFWErrorCallback);
        }

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
        ++s_GLFWWindowCount;

        m_Context = GraphicsContext::Create(m_Window);
        m_Context->Init();

        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                data.Width = width;
                data.Height = height;

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
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, true);
                    data.EventCallback(event);
                    break;
                }
                }
            });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                KeyTypedEvent event(keycode);
                data.EventCallback(event);
            });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                switch (action)
                {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
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
    }

    void MacWindow::Shutdown()
    {
        glfwDestroyWindow(m_Window);
        --s_GLFWWindowCount;

        if (s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
    }

    void MacWindow::OnUpdate()
    {
        glfwPollEvents();
        m_Context->SwapBuffers();
    }

    void MacWindow::SetVSync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool MacWindow::IsVSync() const
    {
        return m_Data.VSync;
    }

}
