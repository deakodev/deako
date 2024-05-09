#pragma once

#include <glm/glm.hpp>

namespace Deak {

    class Camera
    {
    public:
        Camera() = default;
        Camera(float fov, float aspectRatio, float nearPlane, float farPlane);
        virtual ~Camera() = default;

        const glm::mat4& GetProjection() const { return m_Projection; }
        const glm::mat4& GetView() const { return m_View; }
        const glm::mat4& GetViewProjection() const { return m_ViewProjection; }

    protected:
        glm::mat4 m_Projection;
        glm::mat4 m_View;
        glm::mat4 m_ViewProjection;
    };



    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(float fov, float aspectRatio, float nearPlane, float farPlane);

        void UpdateProjection(float fov, float aspectRatio, float nearPlane, float farPlane);
        void UpdateView(const glm::vec3& position, const glm::quat& orientation);
    };

}
