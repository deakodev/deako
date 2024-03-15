#include "Deak.h"

class ExampleLayer : public Deak::Layer
{
public:
    ExampleLayer()
        : Layer("Example")
    {
    }

    void OnUpdate() override
    {
        DK_INFO("ExampleLayer::Update");

        if (Deak::Input::IsKeyPressed(Deak::Key::Up))
            DK_INFO("Up key is pressed");

        if (Deak::Input::IsKeyPressed(Deak::Key::Space))
            DK_INFO("Space key is pressed");

        if (Deak::Input::IsMouseButtonPressed(Deak::Mouse::ButtonLeft))
            DK_INFO("Left mouse button is pressed");

        if (Deak::Input::IsMouseButtonPressed(Deak::Mouse::ButtonRight))
            DK_INFO("Right mouse button is pressed");
    }

    void OnEvent(Deak::Event& event) override
    {
        // DK_TRACE("{0}", event);
    }
};

class Sandbox : public Deak::Application
{
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
        PushOverlay(new Deak::ImGuiLayer());
    }

    ~Sandbox()
    {
    }
};

Deak::Application* Deak::CreateApplication()
{
    return new Sandbox();
}

