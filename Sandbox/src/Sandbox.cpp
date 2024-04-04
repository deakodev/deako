#include "Deak.h"
#include "Deak/Core/EntryPoint.h"

#include "imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Example2D.h"
#include "Example3D.h"

class Sandbox : public Deak::Application
{
public:
    Sandbox()
    {
        // PushLayer(new Example2D());
        PushLayer(new Example3D());
    }

    ~Sandbox()
    {
    }
};

Deak::Application* Deak::CreateApplication()
{
    return new Sandbox();
}

