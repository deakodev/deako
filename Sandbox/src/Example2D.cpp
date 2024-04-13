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
    DK_PROFILE_FUNC();

    m_BoxTexture = Deak::Texture2D::Create("Sandbox/assets/textures/container.jpg");
}

void Example2D::OnDetach()
{
    DK_PROFILE_FUNC();
}

void Example2D::OnUpdate(Deak::Timestep timestep)
{
    DK_PROFILE_FUNC();

    m_CameraController.OnUpdate(timestep);

    Deak::Renderer2D::ResetStats();
    {
        DK_PROFILE_SCOPE("Renderer Prep");
        Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Deak::RenderCommand::Clear();
    }
    {
        DK_PROFILE_SCOPE("Renderer Draw");
        Deak::Renderer2D::BeginScene(m_CameraController.GetCamera());
        Deak::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
        Deak::Renderer2D::DrawRotQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, -45.0f, { 0.8f, 0.2f, 0.3f, 1.0f });
        Deak::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
        Deak::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, m_BoxTexture, 10.0f);
        Deak::Renderer2D::DrawRotQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, 45.0f, m_BoxTexture, 1.0f, { 1.0f, 0.2f, 0.3f, 1.0f });
        Deak::Renderer2D::EndScene();

        Deak::Renderer2D::BeginScene(m_CameraController.GetCamera());
        for (float y = -5.0f; y < 5.0f; y += 0.5f)
        {
            for (float x = -5.0f; x < 5.0f; x += 0.5f)
            {
                glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
                Deak::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
            }
        }
        Deak::Renderer2D::EndScene();
    }
}

void Example2D::OnEvent(Deak::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Example2D::OnImGuiRender()
{
    DK_PROFILE_FUNC();

    auto stats = Deak::Renderer2D::GetStats();
    ImGui::Begin("Render2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quad Count: %d", stats.quadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::End();
}
