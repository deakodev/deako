#pragma once

#include "dkpch.h"
#include "Deako/Core/Base.h"
#include "Deako/Events/Event.h"

namespace Deako {

    using EventCallbackFn = std::function<void(Event&)>;

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

        virtual void OnUpdate() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        // Useful to obtain GLFW window pointer
        virtual void* GetNativeWindow() const = 0;

        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;
        virtual std::pair<int, int> GetWindowFramebufferSize() = 0;

        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    };

}
