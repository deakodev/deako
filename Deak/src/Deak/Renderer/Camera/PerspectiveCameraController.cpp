#include "PerspectiveCameraController.h"
#include "dkpch.h"

#include "Deak/Core/Input.h"
#include "Deak/Core/KeyCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Deak {

    PerspectiveCameraController::PerspectiveCameraController(float fov, float viewportWidth, float viewportHeight, float nearPlane, float farPlane)
        : m_Camera(fov, viewportWidth / viewportHeight, nearPlane, farPlane)
        , m_ViewportWidth(viewportWidth)
        , m_ViewportHeight(viewportHeight)
    {
        CalculatePanSpeed();
    }

    void PerspectiveCameraController::OnUpdate(Timestep timestep)
    {
        DK_PROFILE_FUNC();

        if (Input::IsKeyPressed(Key::Space))
        {
            const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
            glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
            m_InitialMousePosition = mouse;

            if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
                HandleMousePan(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
                HandleMouseRotate(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
                HandleMouseZoom(delta.y);
        }

        if (Input::IsKeyPressed(Key::A))
        {
            m_FocalPoint.x += m_TranslationSpeed * timestep;
        }
        else if (Input::IsKeyPressed(Key::D))
        {
            m_FocalPoint.x -= m_TranslationSpeed * timestep;
        }

        if (Input::IsKeyPressed(Key::W))
        {
            m_FocalPoint.y -= m_TranslationSpeed * timestep;
        }
        else if (Input::IsKeyPressed(Key::S))
        {
            m_FocalPoint.y += m_TranslationSpeed * timestep;
        }

        m_Camera.UpdateView(UpdatePosition(), GetOrientation());
    }

    void PerspectiveCameraController::OnEvent(Event& event)
    {
        DK_PROFILE_FUNC();

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrolledEvent>(DK_BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(DK_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
    }

    void PerspectiveCameraController::SetViewportSize(float width, float height)
    {
        DK_PROFILE_FUNC();

        m_ViewportWidth = width;
        m_ViewportHeight = height;
        HandleCameraResize(m_ViewportWidth, m_ViewportHeight);
        CalculatePanSpeed();
    }

    bool PerspectiveCameraController::OnMouseScrolled(MouseScrolledEvent& event)
    {
        DK_PROFILE_FUNC();

        float delta = event.GetYOffset() * 0.1f;
        HandleMouseZoom(delta);
        m_Camera.UpdateView(UpdatePosition(), GetOrientation());
        return false;
    }

    void PerspectiveCameraController::HandleMouseZoom(float delta)
    {
        DK_PROFILE_FUNC();

        m_Distance -= delta * CalculateZoomSpeed();
        if (m_Distance < 1.0f)
        {
            m_FocalPoint += GetForwardDirection();
            m_Distance = 1.0f;
        }
    }

    bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& event)
    {
        DK_PROFILE_FUNC();

        HandleCameraResize((float)event.GetWidth(), (float)event.GetHeight());
        return false;
    }

    void PerspectiveCameraController::HandleCameraResize(float width, float height)
    {
        DK_PROFILE_FUNC();

        m_Camera.UpdateProjection(m_FOV, width / height, m_NearPlane, m_FarPlane);
    }

    void PerspectiveCameraController::HandleMousePan(const glm::vec2& delta)
    {
        DK_PROFILE_FUNC();

        auto [xSpeed, ySpeed] = m_PanSpeed;
        m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
        m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
    }

    void PerspectiveCameraController::HandleMouseRotate(const glm::vec2& delta)
    {
        DK_PROFILE_FUNC();

        float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yawSign * delta.x * m_RotationSpeed;
        m_Pitch += delta.y * m_RotationSpeed;
    }

    void PerspectiveCameraController::CalculatePanSpeed()
    {
        DK_PROFILE_FUNC();

        float x = std::min(m_ViewportWidth / 1000.0f, 2.4f);
        float xPanSpeed = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = std::min(m_ViewportHeight / 1000.0f, 2.4f);
        float yPanSpeed = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        m_PanSpeed = { xPanSpeed, yPanSpeed };
    }

    float PerspectiveCameraController::CalculateZoomSpeed() const
    {
        DK_PROFILE_FUNC();

        float distance = m_Distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 100.0f);
        return speed;
    }

    glm::vec3 PerspectiveCameraController::UpdatePosition() const
    {
        DK_PROFILE_FUNC();

        return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    glm::quat PerspectiveCameraController::GetOrientation() const
    {
        DK_PROFILE_FUNC();

        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }

    glm::vec3 PerspectiveCameraController::GetUpDirection() const
    {
        DK_PROFILE_FUNC();

        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 PerspectiveCameraController::GetRightDirection() const
    {
        DK_PROFILE_FUNC();

        return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 PerspectiveCameraController::GetForwardDirection() const
    {
        DK_PROFILE_FUNC();

        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

}

