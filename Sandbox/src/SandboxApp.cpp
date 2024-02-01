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

int main()
{
    Sandbox* sandbox = new Sandbox();
    sandbox->Run();
    delete sandbox;

    return 0;
}
