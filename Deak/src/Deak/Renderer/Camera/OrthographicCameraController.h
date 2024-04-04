#pragma once

#include "Camera.h"

#include "Deak/Events/ApplicationEvent.h"
#include "Deak/Events/MouseEvent.h"
#include "Deak/Core/Timestep.h"

namespace Deak {

    // Default camera / controller specs
    const glm::vec3 O_POSITION = { 0.0f, 0.0f, 0.0f };
    const bool ENABLE_ROTATION = true;
    const float ROTATION = 0.0f; // Degrees
    const float AR = 1.778f;
    const float ZOOM_LEVEL = 1.0f;
    const float O_TRANS_SPEED = 5.0f;
    const float O_ROT_SPEED = 180.0f;

    class OrthographicCameraController
    {
    public:
        OrthographicCameraController(bool enableRotation = ENABLE_ROTATION, float aspectRatio = AR, float zoomLevel = ZOOM_LEVEL);

        void OnUpdate(Timestep timestep);
        void OnEvent(Event& event);
        void SetViewportSize(float width, float height);

        OrthographicCamera& GetCamera() { return m_Camera; }
        const OrthographicCamera& GetCamera() const { return m_Camera; }

        float GetZoomLevel() const { return m_ZoomLevel; }
        void SetZoomLevel(float level) { m_ZoomLevel = level; }

    private:
        bool OnMouseScrolled(MouseScrolledEvent& event);
        bool OnWindowResized(WindowResizeEvent& event);

    private:
        glm::vec3 m_Position = O_POSITION;

        bool m_EnableRotation = ENABLE_ROTATION;
        float m_Rotation = ROTATION;
        float m_AspectRatio = AR;
        float m_ZoomLevel = ZOOM_LEVEL;

        // Movement/Sensitivity
        float m_TranslationSpeed = O_TRANS_SPEED;
        float m_RotationSpeed = O_ROT_SPEED;

        OrthographicCamera m_Camera;
    };

}
