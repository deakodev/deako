#pragma once

#include <glm/glm.hpp>

namespace Deak {

    enum ProjectionType { Orthographic = 0, Perspective = 1 };
    class SceneCamera
    {
    public:
        using UpdateProjectionFn = void (SceneCamera::*)();

        SceneCamera(ProjectionType projectionType);

        void SetProjectionType(ProjectionType type);
        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        void SetOrthographic(float size, float nearClip, float farClip);
        void SetPerspective(float verticalFov, float nearClip, float farClip);
        const glm::mat4& GetProjection() const { return m_Projection; }

        void SetViewportSize(float width, float height);

        void SetOrthographicSize(float size) { m_OrthographicSize = size; UpdateProjection(); }
        float GetOrthographicSize() const { return m_OrthographicSize; }
        void SetOrthographicNear(float nearClip) { m_OrthographicNear = nearClip; UpdateProjection(); }
        float GetOrthographicNear() const { return m_OrthographicNear; }
        void SetOrthographicFar(float farClip) { m_OrthographicFar = farClip; UpdateProjection(); }
        float GetOrthographicFar() const { return m_OrthographicFar; }

        void SetPerspectiveVerticalFov(float verticalFov) { m_PerspectiveFov = verticalFov; UpdateProjection(); }
        float GetPerspectiveVerticalFov() const { return m_PerspectiveFov; }
        void SetPerspectiveNear(float nearClip) { m_PerspectiveNear = nearClip; UpdateProjection(); }
        float GetPerspectiveNear() const { return m_PerspectiveNear; }
        void SetPerspectiveFar(float farClip) { m_PerspectiveFar = farClip; UpdateProjection(); }
        float GetPerspectiveFar() const { return m_PerspectiveFar; }

    private:
        void SetUpdateFunction();
        void UpdateProjection();
        void UpdateOrthographicProjection();
        void UpdatePerspectiveProjection();

    private:
        ProjectionType m_ProjectionType;
        UpdateProjectionFn m_UpdateProjectionFn;
        glm::mat4 m_Projection = glm::mat4(1.0f);

        float m_OrthographicSize = 10.0f;
        float m_OrthographicNear = -1.0f;
        float m_OrthographicFar = 1.0f;

        float m_PerspectiveFov = glm::radians(45.0f);
        float m_PerspectiveNear = 0.01f;
        float m_PerspectiveFar = 1000.0f;

        float m_AspectRatio = 0.0f;
    };

}
