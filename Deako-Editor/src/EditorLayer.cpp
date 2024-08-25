#include "EditorLayer.h"

namespace Deako {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        m_ActiveScene = Scene::GetActiveScene();

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnDetach()
    {

    }

    void EditorLayer::OnUpdate()
    {
        m_ViewportPanel.OnUpdate();

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

        //// SCENE HIERARCHY ////
        m_SceneHierarchyPanel.OnImGuiRender();

        //// VIEWPORT ////
        m_ViewportPanel.OnImGuiRender(textureID);

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& event)
    {
    }

}
