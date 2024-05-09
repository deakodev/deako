#pragma once

#include "EditorCamera.h"

#include "Deak/Events/ApplicationEvent.h"
#include "Deak/Events/MouseEvent.h"
#include "Deak/Core/Timestep.h"

namespace Deak {

    class EditorCameraController
    {
    public:
        EditorCameraController();
        EditorCameraController(float fov, float viewportWidth, float viewportHeight, float nearClip, float farClip);

        void OnUpdate(Timestep timestep);
        void OnEvent(Event& event);
        void SetViewportSize(float width, float height);

        EditorCamera& GetCamera() { return m_Camera; }
        const EditorCamera& GetCamera() const { return m_Camera; }

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
        EditorCamera m_Camera;

        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };
        glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

        // Projection frustum values
        float m_FOV = 40.0f;
        float m_ViewportWidth = 1.0f;
        float m_ViewportHeight = 1.0f;
        float m_NearClip = 0.1f;
        float m_FarClip = 10000.0f;

        // View orientation values
        float m_Distance = 15.0f;
        float m_Pitch = 0.0f;
        float m_Yaw = 0.0f;

        // Movement/Sensitivity
        float m_TranslationSpeed = 5.0f;
        float m_RotationSpeed = 0.8f;
        std::pair<float, float> m_PanSpeed = { 0.0f, 0.0f };
    };

}
