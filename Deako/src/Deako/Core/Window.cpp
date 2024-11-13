#include "Window.h"
#include "dkpch.h"

#include "Deako/Event/KeyEvent.h"

namespace Deako {

    static void GLFWErrorCallback(int error, const char* description)
    {
        DK_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    DkWindow::DkWindow(DkContext* context, DkWindowConfig& config)
    {
        DK_CORE_ASSERT(context, "DkWindow requires a DkContext!");
        DK_CORE_ASSERT(!context->window, "DkWindow already created for DkContext!");

        int success = glfwInit();
        DK_CORE_ASSERT(success, "GLFW was not initialized!");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // set to none for vulkan
        glfwSetErrorCallback(GLFWErrorCallback);

        this->context = context;
        this->active = true;
        this->name = config.windowName;
        DK_CORE_INFO("Creating {0} (DkWindow)", config.windowName);

        this->size = config.size;
        DK_CORE_INFO("DkWindow Size: ({0}, {1})", this->size.x, this->size.y);

        this->glfwWindow = glfwCreateWindow((int)this->size.x, (int)this->size.y, this->name, nullptr, nullptr);

        // store dpi scale
        glfwGetWindowContentScale(this->glfwWindow, &this->dpiScale.x, &this->dpiScale.y);

        // Gives easy access to the windows' data in the callbacks via a pointer
        glfwSetWindowUserPointer(this->glfwWindow, this);

        // GLFW Callbacks
        this->EventCallback = DK_BIND_EVENT_FN(DkWindow::OnEvent);

        glfwSetWindowSizeCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, int width, int height)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);
                deakoWindow.size.x = width;
                deakoWindow.size.y = height;

                WindowResizeEvent event(width, height);
                deakoWindow.EventCallback(event);
            });

        glfwSetWindowIconifyCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, int iconified)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);

                if (iconified)
                {
                    WindowMinimizedEvent event{};
                    deakoWindow.EventCallback(event);
                }
                else
                {
                    WindowRestoredEvent event{};
                    deakoWindow.EventCallback(event);
                }
            });

        glfwSetWindowCloseCallback(this->glfwWindow, [](GLFWwindow* glfwWindow)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);
                WindowCloseEvent event{};
                deakoWindow.EventCallback(event);
            });

        glfwSetKeyCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, int keycode, int scancode, int action, int mods)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);

                if (action == GLFW_PRESS)
                {
                    KeyPressedEvent event(static_cast<KeyCode>(keycode), 0);
                    deakoWindow.EventCallback(event);
                }
                else if (action == GLFW_RELEASE)
                {
                    KeyReleasedEvent event(static_cast<KeyCode>(keycode));
                    deakoWindow.EventCallback(event);
                }
                else if (action == GLFW_REPEAT)
                {
                    KeyPressedEvent event(static_cast<KeyCode>(keycode), 1);
                    deakoWindow.EventCallback(event);
                }
            });

        glfwSetCharCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, unsigned int keycode)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);

                KeyTypedEvent event(static_cast<KeyCode>(keycode));
                deakoWindow.EventCallback(event);
            });

        glfwSetMouseButtonCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, int button, int action, int mods)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);

                if (action == GLFW_PRESS)
                {
                    MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                    deakoWindow.EventCallback(event);
                }
                else if (action == GLFW_RELEASE)
                {
                    MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
                    deakoWindow.EventCallback(event);
                }
            });

        glfwSetScrollCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, double xOffset, double yOffset)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);

                MouseScrolledEvent event((DkF32)xOffset, (DkF32)yOffset);
                deakoWindow.EventCallback(event);
            });

        glfwSetCursorPosCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, DkF64 xPos, DkF64 yPos)
            {
                // MacOs specific fix to prevent mouse moved event from firing when cursor is outside of window
                if (glfwGetWindowAttrib(glfwWindow, GLFW_HOVERED))
                {
                    DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);

                    MouseMovedEvent event((DkF32)xPos, (DkF32)yPos);
                    deakoWindow.EventCallback(event);
                }
            });

        // glfwSetFramebufferSizeCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, int width, int height)
        //     {
        //         // VulkanBase::GetState()->framebufferResized = true;

        //         // WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        //         // data.width = width;
        //         // data.height = height;
        //     });

        glfwSetWindowContentScaleCallback(this->glfwWindow, [](GLFWwindow* glfwWindow, float xScale, float yScale)
            {
                DkWindow& deakoWindow = *(DkWindow*)glfwGetWindowUserPointer(glfwWindow);
                deakoWindow.dpiScale = { xScale, yScale };
            });

    }

    DkWindow::~DkWindow()
    {
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
    }

    void DkWindow::OnUpdate()
    {
        glfwPollEvents();
    }

    void DkWindow::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(DK_BIND_EVENT_FN(DkWindow::OnClose));
        dispatcher.Dispatch<WindowMinimizedEvent>(DK_BIND_EVENT_FN(DkWindow::OnMinimized));
        dispatcher.Dispatch<WindowRestoredEvent>(DK_BIND_EVENT_FN(DkWindow::OnRestored));

        context->application->layerStack.OnEvent(event);
    }

    bool DkWindow::OnClose(WindowCloseEvent& event)
    {
        this->active = false;
        context->application->running = false;
        return true;
    }

    bool DkWindow::OnMinimized(WindowMinimizedEvent& event)
    {
        this->active = false;
        return true;
    }

    bool DkWindow::OnRestored(WindowRestoredEvent& event)
    {
        this->active = true;
        return true;
    }


    DkVec2 DkWindow::GetScaledSize()
    {
        DkF32 width = size.x * dpiScale.x;
        DkF32 height = size.y * dpiScale.y;
        return { width, height };
    }



}
