#include "SceneCamera.h"
#include "dkpch.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Deak {

    SceneCamera::SceneCamera(ProjectionType projectionType)
        : m_ProjectionType(projectionType)
    {
        SetUpdateFunction();
    }

    void SceneCamera::SetProjectionType(ProjectionType type)
    {
        if (m_ProjectionType != type) {
            m_ProjectionType = type;
            SetUpdateFunction();
        }
    }

    void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
    {
        m_OrthographicSize = size;
        m_OrthographicNear = nearClip;
        m_OrthographicFar = farClip;
        UpdateProjection();
    }
    void SceneCamera::SetPerspective(float verticalFov, float nearClip, float farClip)
    {
        m_PerspectiveFov = verticalFov;
        m_PerspectiveNear = nearClip;
        m_PerspectiveFar = farClip;
        UpdateProjection();
    }

    void SceneCamera::SetViewportSize(float width, float height)
    {
        if (width > 0.0f && height > 0.0f)
            m_AspectRatio = width / height;
        UpdateProjection();
    }

    void SceneCamera::SetUpdateFunction()
    {
        if (m_ProjectionType == Orthographic)
            m_UpdateProjectionFn = &SceneCamera::UpdateOrthographicProjection;
        else
            m_UpdateProjectionFn = &SceneCamera::UpdatePerspectiveProjection;

        UpdateProjection();
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
        m_Projection = glm::perspective(m_PerspectiveFov,
            m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
    }

}
