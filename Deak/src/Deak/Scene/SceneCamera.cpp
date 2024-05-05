#include "SceneCamera.h"
#include "dkpch.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    SceneCamera::SceneCamera(ProjectionType projectionType)
        : m_ProjectionType(projectionType)
    {
        SetUpdateFunction();
        UpdateProjection();
    }

    void SceneCamera::SetProjectionType(ProjectionType type)
    {
        if (m_ProjectionType != type) {
            m_ProjectionType = type;
            SetUpdateFunction();
        }
    }

    void SceneCamera::SetProjection(float sizeOrFov, float nearPlane, float farPlane)
    {
        if (m_ProjectionType == Orthographic)
        {
            m_OrthographicSize = sizeOrFov;
            m_OrthographicNear = nearPlane;
            m_OrthographicFar = farPlane;
        }
        else
        {
            m_PerspectiveFov = sizeOrFov;
            m_PerspectiveNear = nearPlane;
            m_PerspectiveFar = farPlane;
        }

        UpdateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
        m_AspectRatio = (float)width / (float)height;
        UpdateProjection();
    }

    void SceneCamera::SetUpdateFunction()
    {
        if (m_ProjectionType == Orthographic)
            m_UpdateProjectionFn = &SceneCamera::UpdateOrthographicProjection;
        else
            m_UpdateProjectionFn = &SceneCamera::UpdatePerspectiveProjection;
    }

    void SceneCamera::UpdateProjection()
    {
        (this->*m_UpdateProjectionFn)();
    }

    void SceneCamera::UpdateOrthographicProjection()
    {
        float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
        float orthoLeft = -orthoRight;
        float orthoTop = m_OrthographicSize * 0.5f;
        float orthoBottom = -orthoTop;

        m_Projection = glm::ortho(orthoLeft, orthoRight,
            orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
    }

    void SceneCamera::UpdatePerspectiveProjection()
    {
        m_Projection = glm::perspective(glm::radians(m_PerspectiveFov),
            m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
    }

}
