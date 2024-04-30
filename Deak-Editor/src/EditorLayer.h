#pragma once

#include "Deak.h"

namespace Deak {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Timestep timestep) override;
        virtual void OnEvent(Event& event) override;

        virtual void OnImGuiRender(Timestep timestep) override;

    private:
        PerspectiveCameraController  m_CameraController;
        Ref<Texture2D> m_BoxTexture;
        Ref<Framebuffer> m_Framebuffer;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        glm::vec3 m_ColorModifier = { 0.75f, 0.5f, 0.25f };

    };

}
