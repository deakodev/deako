#include "Application.h"
#include "dkpch.h"

namespace Deako
{

    Application* Application::s_Instance = nullptr;

    Application::Application(const char* name)
    {
        // DK_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;
    }

    Application::~Application()
    {
    }

    void Application::Run()
    {
        std::cout << "Hello world" << '\n';
    }

}
