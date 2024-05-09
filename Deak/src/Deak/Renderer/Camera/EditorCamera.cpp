#include "EditorCamera.h"
#include "dkpch.h"

#include "Deak/Core/Input.h"
#include "Deak/Core/KeyCodes.h"
#include "Deak/Core/MouseCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deak {

    Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
        : m_Projection(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
        , m_View(1.0f)
    {
        DK_PROFILE_FUNC();

        m_ViewProjection = m_Projection * m_View;
    }

    /////EditorCamera ////

    EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
        : Camera(fov, aspectRatio, nearClip, farClip)
    {
        DK_PROFILE_FUNC();
    }

    void EditorCamera::UpdateProjection(float fov, float aspectRatio, float nearClip, float farClip)
    {
        DK_PROFILE_FUNC();

        m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
        m_ViewProjection = m_Projection * m_View;
    }

    void EditorCamera::UpdateView(const glm::vec3& position, const glm::quat& orientation)
    {
        DK_PROFILE_FUNC();

        m_View = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
        m_View = glm::inverse(m_View);
        m_ViewProjection = m_Projection * m_View;
    }

}
