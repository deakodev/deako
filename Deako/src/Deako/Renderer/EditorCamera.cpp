#include "EditorCamera.h"
#include "dkpch.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

    EditorCamera::EditorCamera()
    {
        SetProjectionType(ProjectionType::Perspective);
    }

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

    void EditorCamera::SetProjectionType(ProjectionType type)
    {
        m_ProjectionType = type;

        // assign the appropriate lambda to ProjectionFn
        if (type == ProjectionType::Perspective)
        {
            m_Controller.DefaultTo(ProjectionType::Perspective);
            ProjectionFn = [this]()
                {
                    DkF32 aspectRatio = m_Controller.GetViewportSize().x / m_Controller.GetViewportSize().y;
                    return glm::perspective(glm::radians(m_Controller.GetFOV()), aspectRatio, m_Controller.GetNearPlane(), m_Controller.GetFarPlane());
                };
        }
        else if (type == ProjectionType::Orthographic)
        {
            m_Controller.DefaultTo(ProjectionType::Orthographic);
            ProjectionFn = [this]()
                {
                    DkF32 aspectRatio = m_Controller.GetViewportSize().x / m_Controller.GetViewportSize().y;
                    DkF32 orthoSize = m_Controller.GetOrthoSize();
                    DkF32 halfWidth = orthoSize * aspectRatio * 0.5f;
                    DkF32 halfHeight = orthoSize * 0.5f;
                    return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, m_Controller.GetNearPlane(), m_Controller.GetFarPlane());
                };
        }
    }

    void EditorCamera::ResizeCamera()
    {
        UpdateProjection();
    }

    void EditorCamera::ResizeCamera(const DkVec2& viewportSize)
    {
        m_Controller.SetViewportSize(viewportSize);
        UpdateProjection();
    }

    void EditorCamera::UpdateProjection()
    {
        if (ProjectionFn) m_Projection = ProjectionFn();
    }

    void EditorCamera::UpdateView(const DkVec3& position, const glm::quat& orientation)
    {
        m_View = glm::translate(DkMat4(1.0f), position) * glm::toMat4(orientation);
        m_View = glm::inverse(m_View);
    }

}
