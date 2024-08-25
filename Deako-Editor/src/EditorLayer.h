#pragma once

#include "Deako.h"

#include "Panels/ViewportPanel.h"
#include "Panels/SceneHierarchyPanel.h"

#include <glm/glm.hpp>
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

    private:
        Camera m_EditorCamera;

        ViewportPanel m_ViewportPanel;
        SceneHierarchyPanel m_SceneHierarchyPanel;

        Ref<Scene> m_ActiveScene;
    };

}
