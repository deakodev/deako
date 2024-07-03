#pragma once

#include "Deako/Core/Base.h"
#include "Deako/Core/Window.h"

#include "Deako/Events/Event.h"
#include "Deako/Events/WindowEvent.h"

int main(int argc, char** argv);

namespace Deako {

    class Application
    {
    public:
        Application(const char* name = "Deako App");
        virtual ~Application();

        void OnEvent(Event& event);

        static Application& Get() { return *s_Instance; }
        Window& GetWindow() { return *m_Window; }

    private:
        friend int ::main(int argc, char** argv);
        void Run();

        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowResize(WindowResizeEvent& event);

    private:
        static Application* s_Instance;
        Scope<Window> m_Window;

        bool m_Running = true;
    };

    // To be defined client side
    Application* CreateApplication();

}
