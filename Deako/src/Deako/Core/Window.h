#pragma once

#include "Deako/Core/LayerStack.h"
#include "Deako/Event/Event.h"
#include "Deako/Event/WindowEvent.h"

#include <GLFW/glfw3.h>


namespace Deako {

    using EventCallbackFn = std::function<void(Event&)>;

    struct DkWindowConfig
    {
        const char* windowName = "Deako Window";
        DkVec2 size = { 1600, 900 };
    };

    struct DkWindow
    {
        const char* name;
        DkVec2 size;
        DkVec2 dpiScale;

        GLFWwindow* glfwWindow; // owned by window
        DkContext* context; // not owned, parent context 

        bool active; // set to true on unless window minimized or closed

        EventCallbackFn EventCallback = [](Event&) {};

        DkWindow(DkContext* context, DkWindowConfig& config);
        ~DkWindow();

        void OnUpdate();
        void OnEvent(Event& event);
        bool OnClose(WindowCloseEvent& event);
        bool OnMinimized(WindowMinimizedEvent& event);
        bool OnRestored(WindowRestoredEvent& event);

        DkVec2 GetScaledSize();
    };

    Scope<DkWindow> ConfigureWindow(DkContext* context);  // defined/configured on the client side

}
