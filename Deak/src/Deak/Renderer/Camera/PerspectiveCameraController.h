#pragma once

#include "Camera.h"

#include "Deak/Events/ApplicationEvent.h"
#include "Deak/Events/MouseEvent.h"
#include "Deak/Core/Timestep.h"

namespace Deak {

    // Default camera / controller specs
    const glm::vec3 P_POSITION = { 0.0f, 0.0f, 0.0f };
    const glm::vec3 FOCAL_POINT = { 0.0f, 0.0f, 0.0f };
    const glm::vec2 IMP = { 0.0f, 0.0f };
    const float FOV = 45.0f;
    const float NEARPLANE = 0.1f;
    const float FARPLANE = 1000.0f;
    const float DISTANCE = 15.0f;
    const float PITCH = 0.0f;
    const float YAW = 0.0f;
    const float V_WIDTH = 1280;
    const float V_HEIGHT = 720;
    const float P_TRANS_SPEED = 5.0f;
    const float P_ROT_SPEED = 0.8f;

    class PerspectiveCameraController
    {
    public:
        PerspectiveCameraController(float fov = FOV, float viewportWidth = V_WIDTH, float viewportHeight = V_HEIGHT, float nearPlane = NEARPLANE, float farPlane = FARPLANE);

        void OnUpdate(Timestep timestep);
        void OnEvent(Event& event);
        void SetViewportSize(float width, float height);

        PerspectiveCamera& GetCamera() { return m_Camera; }
        const PerspectiveCamera& GetCamera() const { return m_Camera; }

        const glm::vec3& GetPosition() const { return m_Position; }
        float GetDistance() const { return m_Distance; }
        glm::quat GetOrientation() const;
        glm::vec3 GetUpDirection() const;
        glm::vec3 GetRightDirection() const;
        glm::vec3 GetForwardDirection() const;

        float GetPitch() const { return m_Pitch; }
        float GetYaw() const { return m_Yaw; }

    private:
        glm::vec3 UpdatePosition() const;

        bool OnWindowResized(WindowResizeEvent& event);
        bool OnMouseScrolled(MouseScrolledEvent& event);

        void HandleCameraResize(float width, float height);
        void HandleMousePan(const glm::vec2& delta);
        void HandleMouseRotate(const glm::vec2& delta);
        void HandleMouseZoom(float delta);

        float CalculateZoomSpeed() const;
        void CalculatePanSpeed();

    private:
        PerspectiveCamera m_Camera;

        glm::vec3 m_Position = P_POSITION;
        glm::vec3 m_FocalPoint = FOCAL_POINT;
        glm::vec2 m_InitialMousePosition = IMP;

        // Projection frustum values
        float m_FOV = FOV;
        float m_ViewportWidth = V_WIDTH;
        float m_ViewportHeight = V_HEIGHT;
        float m_NearPlane = NEARPLANE;
        float m_FarPlane = FARPLANE;

        // View orientation values
        float m_Distance = DISTANCE;
        float m_Pitch = PITCH;
        float m_Yaw = YAW;

        // Movement/Sensitivity
        float m_TranslationSpeed = P_TRANS_SPEED;
        float m_RotationSpeed = P_ROT_SPEED;
        std::pair<float, float> m_PanSpeed;

    };

}
