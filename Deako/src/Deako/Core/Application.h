#pragma once

#include "Base.h"

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
    };

    // To be defined client side
    Application* CreateApplication();

}
