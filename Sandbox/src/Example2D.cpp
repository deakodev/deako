#include "Example2D.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Example2D::Example2D()
    : Layer("Example2D"), m_Camera(-10.0f, 10.0f, -10.0f, 10.0f)
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

    // Deak::Renderer::ResetStats();
    {
        DK_PROFILE_SCOPE("Renderer Prep");
        // Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        // Deak::RenderCommand::Clear();
    }
    {
        DK_PROFILE_SCOPE("Renderer Draw");
        Deak::Renderer::BeginScene(m_Camera);

        Deak::Renderer2D::DrawQuad({ 0.0f, -9.0f }, { 5.0f, 0.5f }, { 0.8f, 0.2f, 0.2f, 0.5f });

        Deak::Renderer::EndScene();
    }
}

void Example2D::OnEvent(Deak::Event& event)
{
}

void Example2D::OnImGuiRender(Deak::Timestep timestep)
{
    DK_PROFILE_FUNC();

    // auto stats = Deak::Renderer::GetRendererStats();
    // ImGui::Begin("Render2D Stats:");
    // ImGui::Text("Draw Calls: %d", stats->drawCalls);
    // ImGui::Text("Primitive Count: %d", stats->primitiveCount);
    // ImGui::Text("Vertices: %d", stats->vertexCount);
    // ImGui::Text("Indices: %d", stats->indexCount);
    // ImGui::Text("");
    // ImGui::Text("%.2f FPS", (1.0f / timestep));
    // ImGui::End();
}
