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

    Deak::FramebufferSpec fbSpec;
    fbSpec.width = 1280;
    fbSpec.height = 720;
    m_Framebuffer = Deak::Framebuffer::Create(fbSpec);

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
        m_Framebuffer->Bind();
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
        m_Framebuffer->Unbind();
    }
}

void Example3D::OnEvent(Deak::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Example3D::OnImGuiRender(Deak::Timestep timestep)
{
    DK_PROFILE_FUNC();

    static bool dockingEnabled = true;
    if (dockingEnabled)
    {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace Demo", &dockingEnabled, window_flags);

        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Exit")) Deak::Application::Get().Close();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        auto stats = Deak::Renderer::GetRendererStats();
        ImGui::Begin("Render2D Stats:");
        ImGui::Text("Draw Calls: %d", stats->drawCalls);
        ImGui::Text("Primitive Count: %d", stats->primitiveCount);
        ImGui::Text("Vertices: %d", stats->vertexCount);
        ImGui::Text("Indices: %d", stats->indexCount);
        ImGui::Text("");
        ImGui::Text("%.2f FPS", (1.0f / timestep));

        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)(intptr_t)textureID, ImVec2{ 1280.0f, 720.0f }, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        ImGui::End();
    }
    else
    {
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
}
