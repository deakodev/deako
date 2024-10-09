#pragma once

#include "Deako/Core/Window.h"
#include "Deako/Event/Event.h"

#include <GLFW/glfw3.h>

namespace Deako {

    struct WindowData
    {
        std::string title;
        uint32_t width;
        uint32_t height;

        EventCallbackFn EventCallback = [](Event&) {};
    };

    class MacWindow : public Window
    {
    public:
        MacWindow(const WindowProps& props);
        virtual ~MacWindow();

        virtual void OnUpdate() override;

        virtual uint32_t GetWidth() const override { return m_WindowData.width; }
        virtual uint32_t GetHeight() const override { return m_WindowData.height; }
        virtual Ref<GLFWwindow> GetNativeWindow() const override { return m_Window; }

        virtual std::pair<uint32_t, uint32_t> GetWindowFramebufferSize() override;

        virtual void SetEventCallback(const EventCallbackFn& callback) override
        {
            m_WindowData.EventCallback = callback;
        }

    private:
        void Init(const WindowProps& props);
        void CleanUp();

    private:
        Ref<GLFWwindow> m_Window;
        WindowData m_WindowData;

        static VkInstance m_VkInstance;
    };

}
