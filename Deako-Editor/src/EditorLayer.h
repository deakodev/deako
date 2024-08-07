#pragma once

#include "Deako.h"

#include <glm/glm.hpp>

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
        virtual void OnImGuiRender() override;

    private:
        Camera m_EditorCamera;

        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        bool m_ViewportResize = false;
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;

        std::vector<void*> m_ImGuiViewportTextureIDs;

        Ref<Scene> m_ActiveScene;
        Entity m_VikingRoomEntityA;
        Entity m_VikingRoomEntityB;

        // Ref<Texture2D> m_VikingRoomTexture;
    };

}
