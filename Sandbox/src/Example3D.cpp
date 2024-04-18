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

    // Update light position
    static float lightAngle = 0;
    lightAngle += timestep * 0.5f; // time * speed
    float lightX = 6.0f * cos(lightAngle); // radius * cos(angle)
    float lightZ = 6.0f * sin(lightAngle); // radius * sin(angle)
    glm::vec3 lightPosition = glm::vec3(lightX, 8.0f, lightZ);

    m_CameraController.OnUpdate(timestep);

    Deak::Renderer::ResetStats();
    {
        DK_PROFILE_SCOPE("Renderer Prep");
        Deak::RenderCommand::SetClearColor({ 0.12f, 0.12f, 0.12f, 1.0f });
        Deak::RenderCommand::Clear();
    }
    {
        DK_PROFILE_SCOPE("Renderer Draw");
        Deak::Renderer::BeginScene(m_CameraController, lightPosition);

        // Wall and Floor
        Deak::Renderer3D::DrawCube({ 0.0f, 1.5f, -4.9f }, { 10.0f, 4.0f, 0.2f }, { 0.332, 0.304, 0.288, 1.0 });
        Deak::Renderer3D::DrawCube({ 0.0f, -0.6f, 0.0f }, { 10.0f, 0.2f, 10.0f }, { 0.332, 0.304, 0.288, 1.0 });

        // Box 
        Deak::Renderer3D::DrawCube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, m_BoxTexture, 1.0f, glm::vec4(1.0f));

        // Light
        Deak::Renderer3D::DrawCube(lightPosition, { 0.5f, 0.5f, 0.5f }, glm::vec4(1.0f));

        Deak::Renderer::EndScene();
    }
}

void Example3D::OnEvent(Deak::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Example3D::OnImGuiRender(Deak::Timestep timestep)
{
    DK_PROFILE_FUNC();

    auto stats = Deak::Renderer::GetRendererStats();
    ImGui::Begin("Render2D Stats:");
    ImGui::Text("Draw Calls: %d", stats->drawCalls);
    ImGui::Text("Primitive Count: %d", stats->primitiveCount);
    ImGui::Text("Vertices: %d", stats->vertexCount);
    ImGui::Text("Indices: %d", stats->indexCount);
    ImGui::Text("");
    ImGui::Text("%.2f FPS", (1.0f / timestep));
    ImGui::End();
}
