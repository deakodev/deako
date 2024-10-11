#include "CameraController.h"
#include "dkpch.h"

#include "Deako/Core/Input.h"

namespace Deako {

    CameraController::CameraController()
    {
        UpdatePanSensitivity();
        UpdatePosition();
    }

    CameraController::CameraController(const glm::vec2& viewportSize)
    {
        SetViewportSize(viewportSize);
        UpdatePosition();
    }

    void CameraController::OnUpdate()
    {
        if (Input::IsKeyPressed(Key::LeftAlt))
        {
            const glm::vec2& mousePositionDelta = DetermineMousePositionDelta();

            if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
                HandleMousePan(mousePositionDelta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
                HandleMouseRotate(mousePositionDelta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
                HandleMouseZoom(mousePositionDelta.y);

            UpdatePosition();
        }
    }

    void CameraController::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrolledEvent>(DK_BIND_EVENT_FN(CameraController::OnMouseScrolled));
        // dispatcher.Dispatch<WindowResizeEvent>(DK_BIND_EVENT_FN(CameraController::OnWindowResized));
    }

    bool CameraController::OnMouseScrolled(MouseScrolledEvent& event)
    {
        if (Input::IsKeyPressed(Key::LeftAlt))
        {
            float mouseYDelta = event.GetYOffset() * 0.1f;
            HandleMouseZoom(mouseYDelta);
        }
        return false;
    }

    void CameraController::SetViewportSize(const glm::vec2& viewportSize)
    {
        m_FrustumProjection.viewportSize = viewportSize;
        UpdatePanSensitivity();
    }

    void CameraController::UpdatePosition()
    {
        m_CameraPosition = m_FrustumTarget.focalPoint - DetermineForwardDirection() * m_FrustumTarget.focalDistance;
    }

    glm::quat CameraController::GetOrientation() const
    {
        return glm::quat(glm::vec3(-m_ViewOrientation.pitch, -m_ViewOrientation.yaw, 0.0f));
    }

    void CameraController::UpdatePanSensitivity()
    {
        float x = std::min(m_FrustumProjection.viewportSize.x / 1000.0f, 2.4f);
        float xSpeed = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = std::min(m_FrustumProjection.viewportSize.y / 1000.0f, 2.4f);
        float ySpeed = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        m_Sensitivity.pan = { xSpeed, ySpeed };
    }

    float CameraController::UpdateZoomSensitivity()
    {
        float distance = m_FrustumTarget.focalDistance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        return std::min(speed, 100.0f);
    }

    void CameraController::HandleMousePan(const glm::vec2& mousePositionDelta)
    {
        m_FrustumTarget.focalPoint += DetermineRightDirection() * mousePositionDelta.x * m_Sensitivity.pan.x * m_FrustumTarget.focalDistance;
        m_FrustumTarget.focalPoint += DetermineUpDirection() * mousePositionDelta.y * m_Sensitivity.pan.y * m_FrustumTarget.focalDistance;
    }

    void CameraController::HandleMouseRotate(const glm::vec2& mousePositionDelta)
    {
        m_ViewOrientation.yaw += -mousePositionDelta.x * m_Sensitivity.rotation;
        m_ViewOrientation.pitch += mousePositionDelta.y * m_Sensitivity.rotation;
    }

    void CameraController::HandleMouseZoom(float mouseYDelta)
    {
        m_FrustumTarget.focalDistance -= mouseYDelta * UpdateZoomSensitivity();
        if (m_FrustumTarget.focalDistance < 1.0f)
        {
            m_FrustumTarget.focalPoint += DetermineForwardDirection();
            m_FrustumTarget.focalDistance = 1.0f;
        }
    }

    glm::vec2 CameraController::DetermineMousePositionDelta()
    {
        const glm::vec2& mousePosition = Input::GetMousePosition();
        glm::vec2 mousePositionDelta = (m_InitialMousePosition - mousePosition) * 0.003f;
        m_InitialMousePosition = mousePosition;

        return mousePositionDelta;
    }

    glm::vec3 CameraController::DetermineUpDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 CameraController::DetermineRightDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 CameraController::DetermineForwardDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

}
