#pragma once

#include "Deak.h"

class Example2D : public Deak::Layer
{
public:
    Example2D();
    virtual ~Example2D() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Deak::Timestep timestep) override;
    virtual void OnEvent(Deak::Event& event) override;

    virtual void OnImGuiRender() override;

private:
    Deak::OrthographicCameraController  m_CameraController;
    Deak::Ref<Deak::Texture2D> m_BoxTexture;
};
