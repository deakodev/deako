#pragma once

#include <glm/glm.hpp>

namespace Deak {

    struct PerspectiveParams {
        float fov, aspectRatio, nearPlane, farPlane;
    };

    struct OrthographicParams {
        float left, right, bottom, top;
    };

    class Camera
    {
    public:

        virtual ~Camera() = default;

        const glm::mat4& GetProjection() const { return m_Projection; }
        const glm::mat4& GetView() const { return m_View; }
        const glm::mat4& GetViewProjection() const { return m_ViewProjection; }

    protected:
        explicit Camera(const PerspectiveParams& params);
        explicit Camera(const OrthographicParams& params);

        glm::mat4 m_Projection;
        glm::mat4 m_View;
        glm::mat4 m_ViewProjection;
    };


    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane);

        void UpdateProjection(float fov, float aspectRatio, float nearPlane, float farPlane);
        void UpdateView(const glm::vec3& position, const glm::quat& orientation);
    };


    class OrthographicCamera : public Camera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top);

        void UpdateProjection(float left, float right, float bottom, float top);
        void UpdateView(const glm::vec3& position, float rotation);
    };

}
