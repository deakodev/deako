#include "Camera.h"
#include "dkpch.h"

#include "Deak/Core/Input.h"
#include "Deak/Core/KeyCodes.h"
#include "Deak/Core/MouseCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deak {

    Camera::Camera(const PerspectiveParams& params)
        : m_Projection(glm::perspective(glm::radians(params.fov), params.aspectRatio, params.nearPlane, params.farPlane))
        , m_View(1.0f)
    {
        DK_PROFILE_FUNC();

        m_ViewProjection = m_Projection * m_View;
    }

    Camera::Camera(const OrthographicParams& params)
        : m_Projection(glm::ortho(params.left, params.right, params.bottom, params.top, -1.0f, 1.0f))
        , m_View(1.0f)
    {
        DK_PROFILE_FUNC();

        m_ViewProjection = m_Projection * m_View;
    }

    ///////////////////////////////////////////////
    ///// PerspectiveCamera ///////////////////////
    ///////////////////////////////////////////////

    PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane)
        : Camera(PerspectiveParams{ fov, aspectRatio, nearPlane, farPlane })
    {
        DK_PROFILE_FUNC();
    }

    void PerspectiveCamera::UpdateProjection(float fov, float aspectRatio, float nearPlane, float farPlane)
    {
        DK_PROFILE_FUNC();

        m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        m_ViewProjection = m_Projection * m_View;
    }

    void PerspectiveCamera::UpdateView(const glm::vec3& position, const glm::quat& orientation)
    {
        DK_PROFILE_FUNC();

        m_View = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
        m_View = glm::inverse(m_View);
        m_ViewProjection = m_Projection * m_View;
    }

    ///////////////////////////////////////////////
    ///// OrthographicCamera //////////////////////
    ///////////////////////////////////////////////

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
        : Camera(OrthographicParams{ left, right, bottom, top })
    {
        DK_PROFILE_FUNC();
    }

    void OrthographicCamera::UpdateProjection(float left, float right, float bottom, float top)
    {
        DK_PROFILE_FUNC();

        m_Projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        m_ViewProjection = m_Projection * m_View;
    }

    void OrthographicCamera::UpdateView(const glm::vec3& position, float rotation)
    {
        DK_PROFILE_FUNC();

        m_View = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1));
        m_View = glm::inverse(m_View);
        m_ViewProjection = m_Projection * m_View;
    }
}
