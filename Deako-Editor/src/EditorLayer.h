#pragma once

#include "Deako.h"

#include "Panels/ViewportPanel.h"
#include "Panels/ScenePanel.h"
#include "Panels/AssetsPanel.h"

#include <imgui/imgui.h>

namespace Deako {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnEvent(Event& event) override;
        virtual void OnImGuiRender(ImTextureID textureID) override;

        static void OpenProject();
        static void OpenProject(const std::filesystem::path& path);
        static void OpenScene();
        static void OpenScene(const std::filesystem::path& path);

    private:
        inline static Scope<ScenePanel> s_ScenePanel;
        inline static Scope<AssetsPanel> s_AssetsPanel;
        inline static Scope<ViewportPanel> s_ViewportPanel;

        inline static Ref<Project> s_ActiveProject;
        inline static Ref<Scene> s_ActiveScene;
    };

}
