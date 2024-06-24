#pragma once

#include "dkpch.h"
#include "Deako/Core/Base.h"

namespace Deako {

    struct WindowProps
    {
        const char* title;
        uint32_t width;
        uint32_t height;

        WindowProps(const char* title = "Deako Engine",
            uint32_t width = 1600,
            uint32_t height = 900)
            : title(title), width(width), height(height)
        {
        }
    };

    class Window
    {
    public:
        static Scope<Window> Create(const WindowProps& props = WindowProps());
        virtual ~Window() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        // Useful to obtain GLFW window pointer
        virtual void* GetNativeWindow() const = 0;

        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;
    };

}
