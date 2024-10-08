#pragma once

#include "Deako.h"

#include "Panels/ViewportPanel.h"
#include "Panels/ScenePanel.h"
#include "Panels/RegistryPanel.h"

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

        static void SetContext();
        static void InvalidateContext() { m_IsContextValid = false; }

    private:
        inline static Scope<ScenePanel> s_ScenePanel;
        inline static Scope<RegistryPanel> s_RegistryPanel;
        inline static Scope<ViewportPanel> s_ViewportPanel;

        inline static Ref<Project> s_ActiveProject;
        inline static Ref<Scene> s_ActiveScene;
        inline static Ref<ProjectAssetPool> s_ProjectAssetPool;

        inline static bool m_IsContextValid{ false };
    };

}
