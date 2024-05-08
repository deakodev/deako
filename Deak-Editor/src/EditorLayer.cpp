#include "EditorLayer.h"

#include "Deak/Scene/SceneSerializer.h"
#include "Deak/Utils/PlatformUtils.h"

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

        m_BoxTexture = Texture2D::Create("Deak-Editor/assets/textures/container.jpg");
        m_HealthBarTexture = Texture2D::Create("Deak-Editor/assets/textures/healthbar.png");

        FramebufferSpec fbSpec;
        fbSpec.width = 1280;
        fbSpec.height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();

        #if 0
        m_BoxEntity = m_ActiveScene->CreateEntity("Box");
        // m_BoxEntity.AddComponent<TextureComponent>(m_BoxTexture);
        m_BoxEntity.AddComponent<SpriteRendererComponent>();

        m_FloorEntity = m_ActiveScene->CreateEntity("Floor");
        m_FloorEntity.AddComponent<ColorComponent>(glm::vec4(0.332, 0.304, 0.288, 1.0));
        auto& floorTransfromComp = m_FloorEntity.GetComponent<TransformComponent>();
        floorTransfromComp.translation = { 0.0f, -0.6f, 0.0f };
        floorTransfromComp.scale = { 10.0f, 0.2f, 10.0f };

        m_HealthBarEntity = m_ActiveScene->CreateEntity("Healthbar");
        m_HealthBarEntity.AddComponent<OverlayComponent>(m_HealthBarTexture);
        auto& healthbarTransfromComp = m_HealthBarEntity.GetComponent<TransformComponent>();
        healthbarTransfromComp.translation = { 0.0, -4.5f, 0.0f };
        healthbarTransfromComp.scale = { 4.0f, 0.35f, 1.0f };

        m_PrimaryCameraEntity = m_ActiveScene->CreateEntity("Camera A");
        auto& primaryCameraComp = m_PrimaryCameraEntity.AddComponent<CameraComponent>(Perspective);
        primaryCameraComp.primary = true;
        auto& primaryTransformComp = m_PrimaryCameraEntity.GetComponent<TransformComponent>();
        primaryTransformComp.translation = { 0.0f, 0.0f, 10.0f };

        m_HUDCameraEntity = m_ActiveScene->CreateEntity("Camera B");
        auto& hudCameraComp = m_HUDCameraEntity.AddComponent<CameraComponent>(Orthographic);
        hudCameraComp.hud = true;

        class CameraController : public ScriptableEntity
        {
        public:
            void OnUpdate(Timestep timestep)
            {
                auto& transformComp = GetComponent<TransformComponent>();
                auto& translation = transformComp.translation;
                auto& rotation = transformComp.rotation;

                if (Input::IsKeyPressed(Key::A))
                    translation.x -= 5.0f * timestep;
                else if (Input::IsKeyPressed(Key::D))
                    translation.x += 5.0f * timestep;

                if (Input::IsKeyPressed(Key::W))
                    transformComp.translation.z -= 5.0f * timestep;
                else if (Input::IsKeyPressed(Key::S))
                    translation.z += 5.0f * timestep;

                if (Input::IsKeyPressed(Key::Up))
                    translation.y += 5.0f * timestep;
                else if (Input::IsKeyPressed(Key::Down))
                    translation.y -= 5.0f * timestep;

                if (Input::IsKeyPressed(Key::Q))
                    rotation.y += 5.0f * timestep;
                else if (Input::IsKeyPressed(Key::E))
                    rotation.y -= 5.0f * timestep;
            }
        };

        m_PrimaryCameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        #endif

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
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

    void EditorLayer::OnImGuiRender(Timestep timestep)
    {
        DK_PROFILE_FUNC();

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
                if (ImGui::MenuItem("New...", "Cmd+N")) { NewScene(); }
                if (ImGui::MenuItem("Open...", "Cmd+O")) { OpenScene(); }
                ImGui::Separator();
                if (ImGui::MenuItem("Save", "Cmd+S")) { SaveScene(); }
                if (ImGui::MenuItem("Save As...", "Cmd+Shift+S")) { SaveSceneAs(); }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit Editor")) { Close(); }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // SCENE HIERARCHY ////
        m_SceneHierarchyPanel.OnImGuiRender();

        //// STATS OVERLAY ////
        bool overlayOpen = true;
        ImGuiWindowFlags overlayflags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        auto stats = Renderer::GetRendererStats();
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::Begin("Renderer stats overlay", &overlayOpen, overlayflags);
        ImGui::Text("Draws: %d", stats->drawCalls); ImGui::SameLine(0.0f, 20.0f);
        ImGui::Text("Primitives: %d", stats->primitiveCount); ImGui::SameLine(0.0f, 20.0f);
        ImGui::Text("Vertices: %d", stats->vertexCount); ImGui::SameLine(0.0f, 20.0f);
        ImGui::Text("Indices: %d", stats->indexCount); ImGui::SameLine(0.0f, 20.0f);
        ImGui::Text("Framerate: %.0f  ", (1.0f / timestep)); ImGui::SameLine(0.0f, -1.0f);
        ImGui::End();

        //// VIEWPORT ////
        bool viewportOpen = true;
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

        // ImGui::ShowDemoWindow();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        m_CameraController.OnEvent(event);

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(DK_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<WindowCloseEvent>(DK_BIND_EVENT_FN(EditorLayer::OnEditorClose));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
    {
        // Shortcuts
        if (event.IsRepeat())
            return false;

        bool super = Input::IsKeyPressed(Key::LeftSuper) || Input::IsKeyPressed(Key::RightSuper); // On mac instead of Cmd
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        switch (event.GetKeyCode())
        {
        case Key::N: if (super) NewScene(); break;
        case Key::O: if (super) OpenScene(); break;
        case Key::S:
            if (super && shift) { SaveSceneAs(); break; }
            if (super && !shift) { SaveScene(); break; }
        default: break;
        }

        return false;
    }

    void EditorLayer::NewScene()
    {
        FileUtils::EmptyFilePath();
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        FileUtils::SetFileSaved(false);
    }

    void EditorLayer::OpenScene()
    {
        const std::string filePath = FileUtils::OpenFile("deak");
        if (!filePath.empty())
        {
            m_ActiveScene = CreateRef<Scene>();
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);

            SceneSerializer serializer{ m_ActiveScene };
            serializer.Deserialize(filePath);
            FileUtils::SetFileSaved(false);
        }
    }

    void EditorLayer::SaveScene()
    {
        const std::string& filePath = FileUtils::GetFilePath();
        if (!filePath.empty())
        {
            SceneSerializer serializer{ m_ActiveScene };
            serializer.Serialize(filePath);
            FileUtils::SetFileSaved(true);
        }
        else
        {
            SaveSceneAs();
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        const std::string& filePath = FileUtils::SaveFile("deak");
        if (!filePath.empty())
        {
            SceneSerializer serializer{ m_ActiveScene };
            serializer.Serialize(filePath);
            FileUtils::SetFileSaved(true);
        }
    }

    void EditorLayer::Close()
    {
        enum class Response { Save = 0, DontSave = 1, Cancel = 2, Close = 3 };
        Response response = Response::Close; // Default

        bool fileSaved = FileUtils::IsCurrentFileSaved();
        if (!fileSaved)
            response = static_cast<Response>(FileUtils::PromptSaveOnClose());

        switch (response)
        {
        case Response::Save: { SaveScene(); Application::Get().Close(); break; }
        case Response::Close:
        case Response::DontSave: { Application::Get().Close(); break; }
        case Response::Cancel: break;
        }
    }

    bool EditorLayer::OnEditorClose(WindowCloseEvent& event)
    {
        Close();
        return true;
    }

}
