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
    }

    void OnEvent(Deak::Event& event) override
    {
        DK_TRACE("{0}", event);
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

