#include "EditorLayer.h"

#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Deako {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        m_ScenePanel = CreateScope<ScenePanel>();
        m_PropertiesPanel = CreateScope<PropertiesPanel>();
        m_RegistryPanel = CreateScope<RegistryPanel>();
        m_ViewportPanel = CreateScope<ViewportPanel>();
        m_DebugPanel = CreateScope<DebugPanel>();

        m_EditorCamera = CreateRef<EditorCamera>();
    }

    void EditorLayer::OnDetach()
    {
        m_EditorCamera.reset();
    }

    void EditorLayer::OnUpdate()
    {
        Deako::GetActiveScene().activeCamera = m_EditorCamera;

        m_ViewportPanel->OnUpdate();

        Deako::NewFrame();
        Deako::Render();
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

        bool blockEvents = true;
        for (ImGuiWindow* window : ImGui::GetCurrentContext()->Windows)
        {
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
                (window->Flags & ImGuiWindowFlags_NoDocking) == 0) // Ignore non-dockable windows
            {
                blockEvents = false;
                break;
            }
        }

        Deako::BlockEvents(blockEvents);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Scene", "Cmd+N"))
                {
                    SceneHandler::NewScene();
                    SceneHandler::RefreshScene();
                }
                if (ImGui::MenuItem("Open...", "Cmd+O"))
                {
                    SceneHandler::OpenScene();
                    SceneHandler::RefreshScene();
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
                    Deako::GetApplication().Shutdown();
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

        //// DEBUG ////
        m_DebugPanel->OnImGuiRender();

        ImGui::End();

        ImGui::PopStyleVar();

        // ImGui::ShowDemoWindow();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        m_EditorCamera->OnEvent(event);

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(DK_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
    {
        if (event.IsRepeat()) return false;

        bool super = Deako::IsKeyPressed(Key::LeftSuper) || Deako::IsKeyPressed(Key::RightSuper); // On mac instead of Cmd
        bool shift = Deako::IsKeyPressed(Key::LeftShift) || Deako::IsKeyPressed(Key::RightShift);
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
