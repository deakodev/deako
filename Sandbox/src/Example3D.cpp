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

    Deak::Renderer3D::ResetStats();
    {
        DK_PROFILE_SCOPE("Renderer Prep");
        Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Deak::RenderCommand::Clear();

    }
    {
        DK_PROFILE_SCOPE("Renderer Draw");
        Deak::Renderer3D::BeginScene(m_CameraController.GetCamera());

        for (float y = -25.0f; y < 25.0f; y += 0.5f)
        {
            for (float x = -25.0f; x < 25.0f; x += 0.5f)
            {
                glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
                Deak::Renderer3D::DrawCube({ x, y, 1.0f }, { 0.25f, 0.25f, 0.25f }, m_BoxTexture, 1.0f, color);
            }
        }

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

    auto stats = Deak::Renderer3D::GetStats();
    ImGui::Begin("Render3D Stats:");
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Cube Count: %d", stats.cubeCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::End();
}
