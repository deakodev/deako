#include "EditorLayer.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Deak {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        DK_PROFILE_FUNC();

        m_BoxTexture = Texture2D::Create("Sandbox/assets/textures/container.jpg");

        FramebufferSpec fbSpec;
        fbSpec.width = 1280;
        fbSpec.height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();

        m_BoxEntity = m_ActiveScene->CreateEntity("Box Entity");
        m_BoxEntity.AddComponent<TextureComponent>(m_BoxTexture);

        m_FloorEntity = m_ActiveScene->CreateEntity("Floor Entity");
        m_FloorEntity.AddComponent<ColorComponent>(glm::vec4(0.332, 0.304, 0.288, 1.0));
        auto& floorTransfromComp = m_FloorEntity.GetComponent<TransformComponent>();
        floorTransfromComp.transform *= (glm::translate(glm::mat4(1.0f), { 0.0f, -0.6f, 0.0f })
            * glm::scale(glm::mat4(1.0f), { 10.0f, 0.2f, 10.0f }));

        m_HealthBarEntity = m_ActiveScene->CreateEntity("Healthbar Entity");
        m_HealthBarEntity.AddComponent<OverlayComponent>(glm::vec4(1.0, 0.304, 0.288, 1.0));
        auto& healthbarTransfromComp = m_HealthBarEntity.GetComponent<TransformComponent>();
        healthbarTransfromComp.transform *= glm::translate(glm::mat4(1.0f), { 0.0, -4.5f, 0.0f })
            * glm::scale(glm::mat4(1.0f), { 5.0f, 0.35f, 1.0f });

        m_PrimaryCameraEntity = m_ActiveScene->CreateEntity("Primary Camera Entity");
        auto& primaryCameraComp = m_PrimaryCameraEntity.AddComponent<CameraComponent>(Perspective);
        primaryCameraComp.primary = true;
        auto& primaryTransformComp = m_PrimaryCameraEntity.GetComponent<TransformComponent>();
        primaryTransformComp.transform *= glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 10.0f });

        m_HUDCameraEntity = m_ActiveScene->CreateEntity("HUD Camera Entity");
        auto& hudCameraComp = m_HUDCameraEntity.AddComponent<CameraComponent>(Orthographic);
        hudCameraComp.hud = true;
        auto& hudTransformComp = m_HUDCameraEntity.GetComponent<TransformComponent>();
        hudTransformComp.transform *= glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 10.0f });
    }

    void EditorLayer::OnDetach()
    {
        DK_PROFILE_FUNC();
    }

    void EditorLayer::OnUpdate(Timestep timestep)
    {
        DK_PROFILE_FUNC();

        // Resize framebuffer
        FramebufferSpec spec = m_Framebuffer->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.width != m_ViewportSize.x || spec.height != m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_CameraController.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        // Update camera
        if (m_ViewportFocused)
            m_CameraController.OnUpdate(timestep);

        // Render setup
        Renderer::ResetStats();
        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({ 0.12f, 0.12f, 0.12f, 1.0f });
        RenderCommand::Clear();

        // Update scene
        m_ActiveScene->OnUpdate(timestep);
        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        m_CameraController.OnEvent(event);
    }

    void EditorLayer::OnImGuiRender(Timestep timestep)
    {
        DK_PROFILE_FUNC();

        static bool dockingEnabled = true;
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

                if (ImGui::MenuItem("Exit")) Application::Get().Close();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        //// SIDE PANEL ////
        ImGui::Begin("Render Stats:");
        auto stats = Renderer::GetRendererStats();
        ImGui::Text("Draw Calls: %d", stats->drawCalls);
        ImGui::Text("Primitive Count: %d", stats->primitiveCount);
        ImGui::Text("Vertices: %d", stats->vertexCount);
        ImGui::Text("Indices: %d", stats->indexCount);
        ImGui::Text("");
        ImGui::Text("%.2f FPS", (1.0f / timestep));

        if (m_BoxEntity)
        {
            ImGui::Separator();
            ImGui::Text("%s", m_BoxEntity.GetComponent<TagComponent>().tag.c_str());
            ImGui::Separator();
        }

        ImGui::Separator();
        ImGui::DragFloat3("Orthographic Transform",
            glm::value_ptr(m_HUDCameraEntity.GetComponent<TransformComponent>().transform[3]));
        ImGui::DragFloat3("Perspective Transform",
            glm::value_ptr(m_PrimaryCameraEntity.GetComponent<TransformComponent>().transform[3]));
        ImGui::Separator();

        {
            auto& camera = m_HUDCameraEntity.GetComponent<CameraComponent>().camera;
            float orthoSize = camera.GetOrthographicSize();
            if (ImGui::DragFloat("Ortho Camera Size", &orthoSize))
                camera.SetOrthographicSize(orthoSize);
        }

        ImGui::End();

        //// VIEWPORT ////
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
        ImGui::Begin("Viewport");

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)(intptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

}
