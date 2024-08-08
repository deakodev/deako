#pragma once

#include "Deako.h"

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

        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        bool m_ViewportResize = false;
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;

        Ref<Scene> m_ActiveScene;
        Entity m_BrokenHelmet;
    };

}
