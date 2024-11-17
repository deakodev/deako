#pragma once

#include "Deako/Event/Event.h"
#include "Deako/Event/MouseEvent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

    struct FrustumTarget
    {   // represents the camera's relationship with a focal target
        DkVec3 focalPoint = { 0.0f, 0.0f, 0.0f };
        DkF32 focalDistance = 15.0f;
    };

    struct FrustumProjection
    {
        DkF32 fov = 40.0f;
        DkF32 orthoSize = 0.0f;
        DkF32 nearPlane = 0.1f;
        DkF32 farPlane = 10000.0f;
        DkVec2 viewportSize{ 0.0f, 0.0f };
    };

    struct ViewOrientation
    {
        DkF32 pitch = 0.0f;
        DkF32 yaw = 0.0f;
    };

    struct MovementSensitivity
    {
        DkF32 translation = 5.0f;
        DkF32 rotation = 0.8f;
        DkVec2 pan = { 0.0f, 0.0f };
    };

    enum class ProjectionType;

    class CameraController
    {
    public:
        CameraController();
        CameraController(const DkVec2& viewportSize);

        void DefaultTo(ProjectionType type);

        void OnUpdate();
        void OnEvent(Event& event);
        bool OnMouseScrolled(MouseScrolledEvent& event);

        void SetFOV(DkF32 fov) { m_FrustumProjection.fov = fov; }
        void SetOrthoSize(DkF32 size) { m_FrustumProjection.orthoSize = size; }
        void SetNearPlane(DkF32 nearPlane) { m_FrustumProjection.nearPlane = nearPlane; }
        void SetFarPlane(DkF32 farPlane) { m_FrustumProjection.farPlane = farPlane; }
        void SetViewportSize(const DkVec2& viewportSize);

        const DkVec3& GetPosition() const { return m_CameraPosition; }
        glm::quat GetOrientation() const;

        DkF32 GetFOV() const { return m_FrustumProjection.fov; }
        DkF32 GetNearPlane() const { return m_FrustumProjection.nearPlane; }
        DkF32 GetFarPlane() const { return m_FrustumProjection.farPlane; }
        DkF32 GetOrthoSize() const { return m_FrustumProjection.orthoSize; }
        const DkVec2& GetViewportSize() const { return m_FrustumProjection.viewportSize; }

    private:
        void UpdatePosition();

        void UpdatePanSensitivity();
        DkF32 UpdateZoomSensitivity();

        void HandleMousePan(const DkVec2& mousePositionDelta);
        void HandleMouseRotate(const DkVec2& mousePositionDelta);
        void HandleMouseZoom(DkF32 mouseYDelta);

        DkVec3 GetUpDirection() const;
        DkVec3 GetRightDirection() const;
        DkVec3 GetForwardDirection() const;

    private:
        DkVec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };

        FrustumTarget m_FrustumTarget;
        FrustumProjection m_FrustumProjection;
        ViewOrientation m_ViewOrientation;
        MovementSensitivity m_Sensitivity;

        DkVec2 m_InitialMousePosition = { 0.0f, 0.0f };
    };

}
