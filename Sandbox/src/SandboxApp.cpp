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
    std::cout << "Hello friend!" << '\n';
    return new Sandbox();
}

