#include "Example2D.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Example2D::Example2D()
    : Layer("Example2D")
{
}

void Example2D::OnAttach()
{
    m_BoxTexture = Deak::Texture2D::Create("Sandbox/assets/textures/container.jpg");
}

void Example2D::OnDetach()
{
}

void Example2D::OnUpdate(Deak::Timestep timestep)
{
    m_CameraController.OnUpdate(timestep);

    Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
    Deak::RenderCommand::Clear();

    Deak::Renderer2D::BeginScene(m_CameraController.GetCamera());

    Deak::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
    Deak::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
    Deak::Renderer2D::DrawQuad({ 0.4f, -0.5f , -0.1f }, { 10.0f, 10.0f }, m_BoxTexture);

    Deak::Renderer2D::EndScene();
}

void Example2D::OnEvent(Deak::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Example2D::OnImGuiRender()
{
    // ImGui::Begin("Example2D");
    // ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
    // ImGui::End();
}
