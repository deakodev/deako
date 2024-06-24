#pragma once

#include "Deako/Core/Base.h"
#include "Deako/Core/Window.h"

int main(int argc, char** argv);

namespace Deako {

    class Application
    {
    public:
        Application(const char* name = "Deako App");
        virtual ~Application();

        static Application& Get() { return *s_Instance; }

    private:
        friend int ::main(int argc, char** argv);
        void Run();

    private:
        static Application* s_Instance;

        Scope<Window> m_Window;
    };

    // To be defined client side
    Application* CreateApplication();

}
