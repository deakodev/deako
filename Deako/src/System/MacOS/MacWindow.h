#pragma once

#include "Deako/Core/Window.h"

#include <GLFW/glfw3.h>

namespace Deako {

    struct WindowData
    {
        const char* title;
        uint32_t width;
        uint32_t height;
        bool vsync;
    };

    class MacWindow : public Window
    {
    public:
        MacWindow(const WindowProps& props);
        virtual ~MacWindow();

        virtual uint32_t GetWidth() const override { return m_WindowData.width; }
        virtual uint32_t GetHeight() const override { return m_WindowData.height; }
        virtual void* GetNativeWindow() const override { return m_Window; }

        virtual bool IsVSync() const override;
        virtual void SetVSync(bool enabled) override;

    private:
        void Init(const WindowProps& props);
        void Shutdown();

    private:
        GLFWwindow* m_Window;
        WindowData m_WindowData;
    };

}
