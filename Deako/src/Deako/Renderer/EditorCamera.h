#pragma once

#include "Deako/Renderer/CameraController.h"

#include <glm/glm.hpp>

namespace Deako {

    class EditorCamera
    {
    public:
        void OnUpdate();
        void OnEvent(Event& event);

        void ResizeCamera(const DkVec2& viewportSize);

        DkMat4& GetView() { return m_View; }
        DkMat4& GetProjection() { return m_Projection; }
        CameraController& GetController() { return m_Controller; }

    private:
        void UpdateProjection(DkF32 fov, DkF32 aspectRatio, DkF32 nearPlane, DkF32 farPlane);
        void UpdateView(const DkVec3& position, const glm::quat& orientation);

    private:
        DkMat4 m_View;
        DkMat4 m_Projection;

        CameraController m_Controller;
    };

}
