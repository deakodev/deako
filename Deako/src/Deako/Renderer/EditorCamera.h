#pragma once

#include "Deako/Renderer/CameraController.h"

#include <glm/glm.hpp>

namespace Deako {

    class EditorCamera
    {
    public:
        void OnUpdate();

        void ResizeCamera(const glm::vec2& viewportSize);

        glm::mat4& GetView() { return m_View; }
        glm::mat4& GetProjection() { return m_Projection; }
        CameraController& GetController() { return m_Controller; }

    private:
        void UpdateProjection(float fov, float aspectRatio, float nearPlane, float farPlane);
        void UpdateView(const glm::vec3& position, const glm::quat& orientation);

    private:
        glm::mat4 m_View;
        glm::mat4 m_Projection;

        CameraController m_Controller;
    };

}
