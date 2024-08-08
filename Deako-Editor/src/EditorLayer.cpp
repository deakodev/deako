#include "EditorLayer.h"

namespace Deako {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        m_ActiveScene = CreateRef<Scene>();

        m_BrokenHelmet = m_ActiveScene->CreateEntity("Broken Helmet");
        m_BrokenHelmet.AddComponent<ModelComponent>("models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf");
        auto& brokenHelmetTransfromComp = m_BrokenHelmet.GetComponent<TransformComponent>();
        brokenHelmetTransfromComp.translation = { 0.0f, 0.0f, 0.0f };
        brokenHelmetTransfromComp.scale = 1.0f;

        m_ActiveScene->Prepare();
    }

    void EditorLayer::OnDetach()
    {

    }

    void EditorLayer::OnUpdate()
    {
        if (m_ViewportResize)
        {
            VulkanBase::ViewportResize(m_ViewportSize);
            m_ViewportResize = false;
        }

        m_ActiveScene->OnUpdateEditor(m_EditorCamera);
    }

    void EditorLayer::OnImGuiRender(ImTextureID textureID)
    {
        static bool dockingEnabled = true;
        static bool fullscreenEnabled = true;
        static bool paddingEnabled = false;

        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
        static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        if (fullscreenEnabled)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
            windowFlags |= ImGuiWindowFlags_NoBackground;

        if (!paddingEnabled)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", &dockingEnabled, windowFlags);

        if (!paddingEnabled)
            ImGui::PopStyleVar();
        if (fullscreenEnabled)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowMinSize.x = 360.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
        }
        style.WindowMinSize.x = 32.0f;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // if (ImGui::MenuItem("New...", "Cmd+N")) { NewScene(); }
                // if (ImGui::MenuItem("Open...", "Cmd+O")) { OpenScene(); }
                // ImGui::Separator();
                // if (ImGui::MenuItem("Save", "Cmd+S")) { SaveScene(); }
                // if (ImGui::MenuItem("Save As...", "Cmd+Shift+S")) { SaveSceneAs(); }
                // ImGui::Separator();
                // if (ImGui::MenuItem("Exit Editor")) { Close(); }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        //// VIEWPORT ////
        bool viewportOpen = true;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
        ImGui::Begin("Viewport");

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y)
            m_ViewportResize = true;

        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 0), ImVec2(1, 1));

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void EditorLayer::OnEvent(Event& event)
    {
    }

}
