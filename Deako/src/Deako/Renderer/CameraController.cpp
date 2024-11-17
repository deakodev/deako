#include "CameraController.h"
#include "dkpch.h"

#include "Deako/Renderer/EditorCamera.h"
#include "Deako/Core/Input.h"

namespace Deako {

    CameraController::CameraController()
    {
        UpdatePanSensitivity();
        UpdatePosition();
    }

    CameraController::CameraController(const DkVec2& viewportSize)
    {
        SetViewportSize(viewportSize);
        UpdatePanSensitivity();
        UpdatePosition();
    }

    void CameraController::DefaultTo(ProjectionType type)
    {
        if (type == ProjectionType::Perspective)
        {
            m_FrustumProjection.fov = 40.0f;
            m_FrustumProjection.orthoSize = 0.0f;
            m_FrustumProjection.nearPlane = 0.1f;
            m_FrustumProjection.farPlane = 10000.0f;
        }
        else if (type == ProjectionType::Orthographic)
        {
            m_FrustumProjection.fov = 0.0f;
            m_FrustumProjection.orthoSize = 10.0f;
            m_FrustumProjection.nearPlane = -1.0f;
            m_FrustumProjection.farPlane = 1.0f;
        }
    }

    void CameraController::OnUpdate()
    {
        if (IsKeyPressed(Key::LeftAlt))
        {
            DkInput& di = Deako::GetInput();
            if (IsMousePressed(Mouse::ButtonMiddle))
                HandleMousePan(di.mousePositionDelta);
            else if (IsMousePressed(Mouse::ButtonLeft))
                HandleMouseRotate(di.mousePositionDelta);
            else if (IsMousePressed(Mouse::ButtonRight))
                HandleMouseZoom(di.mousePositionDelta.y);

            UpdatePosition();
        }
    }

    void CameraController::OnEvent(Event& event)
    {
        if (IsKeyPressed(Key::LeftAlt))
        {
            EventDispatcher dispatcher(event);
            dispatcher.Dispatch<MouseScrolledEvent>(DK_BIND_EVENT_FN(CameraController::OnMouseScrolled));
            // dispatcher.Dispatch<WindowResizeEvent>(DK_BIND_EVENT_FN(CameraController::OnWindowResized));
        }
    }

    bool CameraController::OnMouseScrolled(MouseScrolledEvent& event)
    {
        DkF32 mouseYDelta = event.GetYOffset() * 0.1f;
        HandleMouseZoom(mouseYDelta);
        return false;
    }

    void CameraController::SetViewportSize(const DkVec2& viewportSize)
    {
        m_FrustumProjection.viewportSize = viewportSize;
        UpdatePanSensitivity();
    }

    void CameraController::UpdatePosition()
    {
        m_CameraPosition = m_FrustumTarget.focalPoint - GetForwardDirection() * m_FrustumTarget.focalDistance;
    }

    glm::quat CameraController::GetOrientation() const
    {
        return glm::quat(DkVec3(-m_ViewOrientation.pitch, -m_ViewOrientation.yaw, 0.0f));
    }

    void CameraController::UpdatePanSensitivity()
    {
        DkF32 x = std::min(m_FrustumProjection.viewportSize.x / 1000.0f, 2.4f);
        DkF32 xSpeed = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        DkF32 y = std::min(m_FrustumProjection.viewportSize.y / 1000.0f, 2.4f);
        DkF32 ySpeed = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        m_Sensitivity.pan = { xSpeed, ySpeed };
    }

    DkF32 CameraController::UpdateZoomSensitivity()
    {
        DkF32 distance = m_FrustumTarget.focalDistance * 0.2f;
        distance = std::max(distance, 0.0f);
        DkF32 speed = distance * distance;
        return std::min(speed, 100.0f);
    }

    void CameraController::HandleMousePan(const DkVec2& mousePositionDelta)
    {
        m_FrustumTarget.focalPoint += GetRightDirection() * mousePositionDelta.x * m_Sensitivity.pan.x * m_FrustumTarget.focalDistance;
        m_FrustumTarget.focalPoint += GetUpDirection() * mousePositionDelta.y * m_Sensitivity.pan.y * m_FrustumTarget.focalDistance;
    }

    void CameraController::HandleMouseRotate(const DkVec2& mousePositionDelta)
    {
        m_ViewOrientation.yaw += -mousePositionDelta.x * m_Sensitivity.rotation;
        m_ViewOrientation.pitch += mousePositionDelta.y * m_Sensitivity.rotation;
    }

    void CameraController::HandleMouseZoom(DkF32 mouseYDelta)
    {
        m_FrustumTarget.focalDistance -= mouseYDelta * UpdateZoomSensitivity();
        if (m_FrustumTarget.focalDistance < 1.0f)
        {
            m_FrustumTarget.focalPoint += GetForwardDirection();
            m_FrustumTarget.focalDistance = 1.0f;
        }
    }

    DkVec3 CameraController::GetUpDirection() const
    {
        return glm::rotate(GetOrientation(), DkVec3(0.0f, 1.0f, 0.0f));
    }

    DkVec3 CameraController::GetRightDirection() const
    {
        return glm::rotate(GetOrientation(), DkVec3(1.0f, 0.0f, 0.0f));
    }

    DkVec3 CameraController::GetForwardDirection() const
    {
        return glm::rotate(GetOrientation(), DkVec3(0.0f, 0.0f, -1.0f));
    }

}
