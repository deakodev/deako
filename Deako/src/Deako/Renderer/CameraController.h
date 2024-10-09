#pragma once

#include "Deako/Event/Event.h"
#include "Deako/Event/MouseEvent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deako {

    struct FrustumTarget
    {   // represents the camera's relationship with a focal target
        glm::vec3 focalPoint = { 0.0f, 0.0f, 0.0f };
        float focalDistance = 15.0f;
    };

    struct FrustumProjection
    {
        float fov = 40.0f;
        float nearPlane = 0.1f;
        float farPlane = 10000.0f;
        glm::vec2 viewportSize{ 0.0f, 0.0f };
    };

    struct ViewOrientation
    {
        float pitch = 0.0f;
        float yaw = 0.0f;
    };

    struct MovementSensitivity
    {
        float translation = 5.0f;
        float rotation = 0.8f;
        glm::vec2 pan = { 0.0f, 0.0f };
    };

    class CameraController
    {
    public:
        CameraController();
        CameraController(const glm::vec2& viewportSize);

        void OnUpdate();
        void OnEvent(Event& event);
        bool OnMouseScrolled(MouseScrolledEvent& event);

        void SetViewportSize(const glm::vec2& viewportSize);

        const glm::vec3& GetPosition() const { return m_CameraPosition; }
        const glm::quat& GetOrientation() const { return m_CameraOrientation; }

        float GetFOV() const { return m_FrustumProjection.fov; }
        float GetNearPlane() const { return m_FrustumProjection.nearPlane; }
        float GetFarPlane() const { return m_FrustumProjection.farPlane; }

    private:
        void UpdatePosition();
        void UpdateOrientation();

        void UpdatePanSensitivity();
        float UpdateZoomSensitivity();

        void HandleMousePan(const glm::vec2& mousePositionDelta);
        void HandleMouseRotate(const glm::vec2& mousePositionDelta);
        void HandleMouseZoom(float mouseYDelta);

        glm::vec2 DetermineMousePositionDelta();
        glm::vec3 DetermineUpDirection() const;
        glm::vec3 DetermineRightDirection() const;
        glm::vec3 DetermineForwardDirection() const;

    private:
        glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
        glm::quat m_CameraOrientation = glm::quat();

        FrustumTarget m_FrustumTarget;
        FrustumProjection m_FrustumProjection;
        ViewOrientation m_ViewOrientation;
        MovementSensitivity m_Sensitivity;

        glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
    };

}
