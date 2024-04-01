#include "OrthographicCameraController.h"
#include "dkpch.h"

#include "Deak/Core/Input.h"
#include "Deak/Core/KeyCodes.h"

namespace Deak {

    OrthographicCameraController::OrthographicCameraController(bool enableRotation, float aspectRatio, float zoomLevel)
        : m_Camera(-aspectRatio * zoomLevel, aspectRatio* zoomLevel, -zoomLevel, zoomLevel)
        , m_EnableRotation(enableRotation)
        , m_AspectRatio(aspectRatio)
        , m_ZoomLevel(zoomLevel)
    {
    }

    void OrthographicCameraController::OnUpdate(Timestep timestep)
    {
        if (Input::IsKeyPressed(Key::A))
        {
            m_Position.x -= cos(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
            m_Position.y -= sin(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
        }
        else if (Input::IsKeyPressed(Key::D))
        {
            m_Position.x += cos(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
            m_Position.y += sin(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
        }

        if (Input::IsKeyPressed(Key::W))
        {
            m_Position.x += -sin(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
            m_Position.y += cos(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
        }
        else if (Input::IsKeyPressed(Key::S))
        {
            m_Position.x -= -sin(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
            m_Position.y -= cos(glm::radians(m_Rotation)) * m_TranslationSpeed * timestep;
        }

        if (m_EnableRotation)
        {
            if (Input::IsKeyPressed(Key::Q))
                m_Rotation += m_RotationSpeed * timestep;
            if (Input::IsKeyPressed(Key::E))
                m_Rotation -= m_RotationSpeed * timestep;

            if (m_Rotation > 180.0f)
                m_Rotation -= 360.0f;
            else if (m_Rotation <= -180.0f)
                m_Rotation += 360.0f;
        }

        m_Camera.UpdateView(m_Position, m_Rotation);

        m_TranslationSpeed = m_ZoomLevel;
    }

    void OrthographicCameraController::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrolledEvent>(DK_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(DK_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
    }

    bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& event)
    {
        m_ZoomLevel -= event.GetYOffset() * 0.25f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        m_Camera.UpdateProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }

    bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& event)
    {
        SetViewportSize((float)event.GetWidth(), (float)event.GetHeight());
        return false;
    }

    void OrthographicCameraController::SetViewportSize(float width, float height)
    {
        m_AspectRatio = width / height;
        m_Camera.UpdateProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
    }

}
