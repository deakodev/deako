#include "EditorCamera.h"
#include "dkpch.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

    EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
        : m_Projection(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
        , m_View(glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)))
    {
        m_ViewProjection = m_Projection * m_View;
    }

    void EditorCamera::UpdateProjection(float fov, float aspectRatio, float nearClip, float farClip)
    {
        m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
        m_Projection[1][1] *= -1;
        m_ViewProjection = m_Projection * m_View;
    }

    void EditorCamera::UpdateView(const glm::vec3& position, const glm::quat& orientation)
    {
        m_View = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
        m_View = glm::inverse(m_View);
        m_ViewProjection = m_Projection * m_View;
    }

    // void Camera::OnUpdate()
    // {
    //     static auto startTime = std::chrono::high_resolution_clock::now();
    //     auto currentTime = std::chrono::high_resolution_clock::now();
    //     float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    //     s_UBO.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //     s_UBO.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //     s_UBO.projection = glm::perspective(glm::radians(45.0f), s_VR->imageExtent.width / (float)s_VR->imageExtent.height, 0.1f, 10.0f);
    //     s_UBO.projection[1][1] *= -1;
    // }

}
