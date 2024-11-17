#pragma once

#include "Deako/Renderer/CameraController.h"

#include <glm/glm.hpp>

namespace Deako {

    using ProjectionFunction = std::function<DkMat4()>;

    enum class ProjectionType { Perspective = 0, Orthographic = 1 }; // TODO: still need to imple ortho

    class EditorCamera
    {
    public:
        EditorCamera();

        void OnUpdate();
        void OnEvent(Event& event);

        void ResizeCamera();
        void ResizeCamera(const DkVec2& viewportSize);

        void SetProjectionType(ProjectionType type);
        ProjectionType GetProjectionType() { return m_ProjectionType; }

        DkMat4& GetView() { return m_View; }
        DkMat4& GetProjection() { return m_Projection; }
        CameraController& GetController() { return m_Controller; }

    private:
        void UpdateProjection();
        void UpdateView(const DkVec3& position, const glm::quat& orientation);

    private:
        ProjectionType m_ProjectionType = ProjectionType::Perspective;
        ProjectionFunction ProjectionFn;

        DkMat4 m_Projection;
        DkMat4 m_View;

        CameraController m_Controller;
    };

}
