#include "EditorLayer.h"

namespace Deako {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        m_EditorCamera = CreateRef<EditorCamera>();
        m_EditorContext = CreateRef<EditorContext>();

        m_ScenePanel = CreateScope<ScenePanel>(m_EditorContext);
        m_PropertiesPanel = CreateScope<PropertiesPanel>(m_EditorContext);
        m_RegistryPanel = CreateScope<RegistryPanel>(m_EditorContext);
        m_ViewportPanel = CreateScope<ViewportPanel>(m_EditorContext, m_EditorCamera);
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate()
    {
        m_ViewportPanel->OnUpdate();

        m_EditorCamera->OnUpdate();

        m_EditorContext->OnUpdate(m_EditorCamera);
    }

    void EditorLayer::OnImGuiRender(ImTextureID textureID)
    {
        static bool dockingEnabled = true;
        static bool fullscreenEnabled = true;
        static bool paddingEnabled = false;

        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_AutoHideTabBar;
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
        style.WindowMinSize.x = 300.0f;
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
                if (ImGui::MenuItem("New Scene", "Cmd+N"))
                {
                    SceneHandler::NewScene();
                    SceneHandler::RefreshScene();
                    m_EditorContext->scene.isValid = false;
                }
                if (ImGui::MenuItem("Open...", "Cmd+O"))
                {
                    SceneHandler::OpenScene();
                    SceneHandler::RefreshScene();
                    m_EditorContext->scene.isValid = false;
                }
                if (ImGui::BeginMenu("Import", "Cmd+I"))
                {
                    if (ImGui::MenuItem("Scene (.dscene)"))
                    {
                        SceneHandler::OpenScene();
                        m_RegistryPanel->Refresh();
                    }
                    if (ImGui::MenuItem("Prefab (.gltf)"))
                    {
                        PrefabHandler::OpenPrefab();
                        m_RegistryPanel->Refresh();
                    }
                    if (ImGui::MenuItem("Mesh (tbd)"))
                    {
                        DK_INFO("Mesh import not implemented!");
                    }

                    if (ImGui::MenuItem("Material (tbd)"))
                    {
                        DK_INFO("Material import not implemented!");
                    }
                    if (ImGui::MenuItem("Texture 2D (.ktx)"))
                    {
                        TextureHandler::OpenTexture2D();
                        m_RegistryPanel->Refresh();
                    }
                    if (ImGui::MenuItem("Texture Cube Map (.ktx)"))
                    {
                        TextureHandler::OpenTextureCubeMap();
                        m_RegistryPanel->Refresh();
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Save", "Cmd+S"))
                {
                    SceneHandler::SaveScene();
                    ProjectHandler::SaveProject();
                }
                if (ImGui::MenuItem("Save As...", "Cmd+Shift+S"))
                {
                    SceneHandler::SaveAsScene();
                    ProjectHandler::SaveProject();
                }
                if (ImGui::MenuItem("Save Copy", "Cmd+Shift+C"))
                {
                    DK_INFO("Save copy not implemented!");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit Editor", "Cmd+Q"))
                {
                    Application::Get().Close();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        //// SCENE ////
        m_ScenePanel->OnImGuiRender();

        //// PROPERTIES ////
        m_PropertiesPanel->OnImGuiRender();

        //// ASSETS ////
        m_RegistryPanel->OnImGuiRender();

        //// VIEWPORT ////
        m_ViewportPanel->OnImGuiRender(textureID);

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        m_EditorCamera->GetController().OnEvent(event);
    }

}
