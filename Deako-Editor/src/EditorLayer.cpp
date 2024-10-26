#include "EditorLayer.h"

#include <ImGuizmo.h>

namespace Deako {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        m_EditorContext = CreateRef<EditorContext>();
        m_EditorCamera = m_EditorContext->scene->activeCamera;

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

        m_EditorContext->OnUpdate();
    }

    void EditorLayer::OnImGuiRender()
    {
        static ImGuiWindowFlags dockingWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(300.0f, 150.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        static bool dockingEnabled = true;
        ImGui::Begin("DockSpace", &dockingEnabled, dockingWindowFlags);

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);

        ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode);

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
        m_ViewportPanel->OnImGuiRender();

        ImGui::End();

        ImGui::PopStyleVar();

        ImGui::ShowDemoWindow();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        m_EditorCamera->GetController().OnEvent(event);

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(DK_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
    {
        if (event.IsRepeat()) return false;

        bool super = Input::IsKeyPressed(Key::LeftSuper) || Input::IsKeyPressed(Key::RightSuper); // On mac instead of Cmd
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        switch (event.GetKeyCode())
        {
            // File Shortcuts
            // case Key::N: if (super) NewScene(); break;
            // case Key::O: if (super) OpenScene(); break;
            // case Key::S:
            //     if (super && shift) { SaveSceneAs(); break; }
            //     if (super && !shift) { SaveScene(); break; }

        // Gizmo Shortcuts
        case Key::Q: m_ViewportPanel->SetGizmoOperation((ImGuizmo::OPERATION)-1); break;
        case Key::W: m_ViewportPanel->SetGizmoOperation(ImGuizmo::OPERATION::TRANSLATE); break;
        case Key::E: m_ViewportPanel->SetGizmoOperation(ImGuizmo::OPERATION::ROTATE); break;
        case Key::R: m_ViewportPanel->SetGizmoOperation(ImGuizmo::OPERATION::SCALE); break;

        default: break;
        }

        return false;
    }

}
