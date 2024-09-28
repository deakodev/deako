#include "EditorLayer.h"

namespace Deako {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        s_AssetsPanel = CreateScope<AssetsPanel>();
        s_ScenePanel = CreateScope<ScenePanel>();
        s_ViewportPanel = CreateScope<ViewportPanel>();

        OpenProject();
        OpenScene();
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate()
    {
        s_ViewportPanel->OnUpdate();
        s_ActiveScene->OnUpdate();
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
                // TODO:
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

        //// ASSETS ////
        s_AssetsPanel->OnImGuiRender();

        //// SCENE ////
        s_ScenePanel->OnImGuiRender();

        //// VIEWPORT ////
        s_ViewportPanel->OnImGuiRender(textureID);

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& event)
    {
    }


    void EditorLayer::OpenProject()
    {
        std::filesystem::path projectPath = Application::Get().GetSpecification().commandLineArgs[1];
        if (!projectPath.empty())
        {
            OpenProject(projectPath);
        }
        else
        {
            DK_ERROR("No intial project selected!");
            Application::Get().Close(); return;
        }
    }

    void EditorLayer::OpenProject(const std::filesystem::path& path)
    {
        s_ActiveProject = Project::Load(path);

        if (s_ActiveProject)
        {
            s_AssetsPanel->SetContext(s_ActiveProject);
        }
        else
        {
            DK_ERROR("No active project selected!");
            Application::Get().Close(); return;
        }
    }

    void EditorLayer::OpenScene()
    {
        std::filesystem::path scenePath = s_ActiveProject->GetInitialScenePath();
        if (!scenePath.empty())
        {
            OpenScene(scenePath);
        }
        else
        {
            DK_ERROR("No intial scene selected!");
            Application::Get().Close(); return;
        }
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        s_ActiveScene = Scene::Open(path);

        if (s_ActiveScene)
        {
            s_ScenePanel->SetContext(s_ActiveScene);
        }
        else
        {
            DK_ERROR("No active scene selected!");
            Application::Get().Close(); return;
        }
    }



}
