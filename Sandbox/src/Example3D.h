#pragma once

#include "Deak.h"

class Example3D : public Deak::Layer
{
public:
    Example3D();
    virtual ~Example3D() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Deak::Timestep timestep) override;
    virtual void OnEvent(Deak::Event& event) override;

    virtual void OnImGuiRender() override;

private:
    Deak::PerspectiveCameraController  m_CameraController;

    //TODO remove once 3D renderer is built
    Deak::Ref<Deak::Shader> m_Shader;
    Deak::Ref<Deak::VertexArray> m_VertexArray;
    glm::vec3 m_ColorModifier = { 0.75f, 0.5f, 0.25f };

};
