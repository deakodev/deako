#pragma once

#include <glm/glm.hpp>

namespace Deak {

    enum ProjectionType { Orthographic, Perspective };

    class SceneCamera
    {
    public:
        using UpdateProjectionFn = void (SceneCamera::*)();

        SceneCamera(ProjectionType projectionType);

        void SetProjectionType(ProjectionType type);
        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        void SetProjection(float sizeOrFov, float nearClip, float farClip);
        const glm::mat4& GetProjection() const { return m_Projection; }

        void SetViewportSize(uint32_t width, uint32_t height);

        void SetOrthographicSize(float size) { m_OrthographicSize = size; UpdateProjection(); }
        float GetOrthographicSize() const { return m_OrthographicSize; }

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

        float m_PerspectiveFov = 45.0f;
        float m_PerspectiveNear = 0.1f;
        float m_PerspectiveFar = 1000.0f;

        float m_AspectRatio = 0.0f;
    };

}
