#include "Example3D.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Example3D::Example3D()
    : Layer("Example3D")
{
}

void Example3D::OnAttach()
{
    DK_PROFILE_FUNC();

    m_BoxTexture = Deak::Texture2D::Create("Sandbox/assets/textures/container.jpg");
}

void Example3D::OnDetach()
{
    DK_PROFILE_FUNC();
}

void Example3D::OnUpdate(Deak::Timestep timestep)
{
    DK_PROFILE_FUNC();

    m_CameraController.OnUpdate(timestep);

    {
        DK_PROFILE_SCOPE("Renderer Prep");
        Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Deak::RenderCommand::Clear();

    }
    {
        DK_PROFILE_SCOPE("Renderer Draw");
        Deak::Renderer3D::BeginScene(m_CameraController.GetCamera());

        Deak::Renderer3D::DrawQuad({ 1.0f, 1.0f }, { 0.8f, 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
        Deak::Renderer3D::DrawQuad({ -1.0f, -1.0f }, { 0.5f, 0.75f, 0.5f }, { 0.2f, 0.3f, 0.8f, 1.0f });
        Deak::Renderer3D::DrawQuad({ 0.0f, 0.0f , 2.0f }, { 1.0f, 1.0f, 1.0f }, m_BoxTexture);

        Deak::Renderer3D::EndScene();
    }
}

void Example3D::OnEvent(Deak::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Example3D::OnImGuiRender()
{
    DK_PROFILE_FUNC();

    // ImGui::Begin("Example3D");
    // ImGui::ColorEdit3("Square Color", glm::value_ptr(m_ColorModifier));
    // ImGui::End();
}
