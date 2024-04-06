#pragma once

#include "Base.h"
#include "Deak/Events/Event.h"
#include "dkpch.h"

namespace Deak {

    struct WindowProps
    {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Deak Engine",
            uint32_t width = 1280,
            uint32_t height = 720)
            : Title(title), Width(width), Height(height)
        {
        }
    };

    // Interface representing a desktop Window
    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        // Window attributes
        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;

        // Will allow us to get pointer to GFLW window
        virtual void* GetNativeWindow() const = 0;

        static Scope<Window> Create(const WindowProps& props = WindowProps());
    };

}
