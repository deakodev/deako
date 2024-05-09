#include "Deak.h"
#include "Deak/Core/EntryPoint.h"

class Sandbox : public Deak::Application
{
public:
    Sandbox()
    {
        // PushLayer(new Example3D());
        // PushLayer(new Example2D());
    }

    ~Sandbox()
    {
    }
};

Deak::Application* Deak::CreateApplication()
{
    return new Sandbox();
}

