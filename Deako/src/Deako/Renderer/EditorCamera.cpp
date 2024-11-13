#include "EditorCamera.h"
#include "dkpch.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

    void EditorCamera::OnUpdate()
    {
        m_Controller.OnUpdate();

        const DkVec3& position = m_Controller.GetPosition();
        const glm::quat& orientation = m_Controller.GetOrientation();

        UpdateView(position, orientation);
    }

    void EditorCamera::OnEvent(Event& event)
    {
        m_Controller.OnEvent(event);
    }

    void EditorCamera::UpdateProjection(DkF32 fov, DkF32 aspectRatio, DkF32 nearPlane, DkF32 farPlane)
    {
        m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        // m_Projection[1][1] *= -1.0f;
    }

    void EditorCamera::UpdateView(const DkVec3& position, const glm::quat& orientation)
    {
        m_View = glm::translate(DkMat4(1.0f), position) * glm::toMat4(orientation);
        m_View = glm::inverse(m_View);
    }

    void EditorCamera::ResizeCamera(const DkVec2& viewportSize)
    {
        m_Controller.SetViewportSize(viewportSize);

        DkF32 fov = m_Controller.GetFOV();
        DkF32 aspectRatio = viewportSize.x / viewportSize.y;
        DkF32 nearPlane = m_Controller.GetNearPlane();
        DkF32 farPlane = m_Controller.GetFarPlane();

        UpdateProjection(fov, aspectRatio, nearPlane, farPlane);
    }

}
