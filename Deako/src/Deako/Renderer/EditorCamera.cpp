#include "EditorCamera.h"
#include "dkpch.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

    void EditorCamera::OnUpdate()
    {
        m_Controller.OnUpdate();

        const glm::vec3& position = m_Controller.GetPosition();
        const glm::quat& orientation = m_Controller.GetOrientation();

        UpdateView(position, orientation);
    }

    void EditorCamera::UpdateProjection(float fov, float aspectRatio, float nearPlane, float farPlane)
    {
        m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        // m_Projection[1][1] *= -1.0f;
    }

    void EditorCamera::UpdateView(const glm::vec3& position, const glm::quat& orientation)
    {
        m_View = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
        m_View = glm::inverse(m_View);
    }

    void EditorCamera::ResizeCamera(const glm::vec2& viewportSize)
    {
        m_Controller.SetViewportSize(viewportSize);

        float fov = m_Controller.GetFOV();
        float aspectRatio = viewportSize.x / viewportSize.y;
        float nearPlane = m_Controller.GetNearPlane();
        float farPlane = m_Controller.GetFarPlane();

        UpdateProjection(fov, aspectRatio, nearPlane, farPlane);
    }

}
