#include "Deak.h"

class Sandbox : public Deak::Application
{
public:
    Sandbox()
    {
    }

    ~Sandbox()
    {
    }

};

Deak::Application* Deak::CreateApplication()
{
    return new Sandbox();
}

