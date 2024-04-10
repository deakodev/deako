#include "Deak.h"
#include "Deak/Core/EntryPoint.h"

#include "GameLayer.h"
#include "Example2D.h"
#include "Example3D.h"

class Sandbox : public Deak::Application
{
public:
    Sandbox()
    {
        PushLayer(new GameLayer());
        // PushLayer(new Example2D());
        // PushLayer(new Example3D());
    }

    ~Sandbox()
    {
    }
};

Deak::Application* Deak::CreateApplication()
{
    return new Sandbox();
}

