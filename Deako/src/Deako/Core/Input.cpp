#include "Input.h"
#include "dkpch.h"

namespace Deako {

    void DkInput::OnUpdate()
    {
        this->UpdateMouse();
    }

    void DkInput::UpdateMouse()
    {
        DkWindow& window = Deako::GetWindow();

        DkF64 xPosition, yPosition;
        glfwGetCursorPos(window.glfwWindow, &xPosition, &yPosition);

        mousePosition.x = xPosition * window.dpiScale.x;
        mousePosition.y = yPosition * window.dpiScale.y;

        if (IsMousePositionValid(mousePosition))
            mousePosition = DkRoundDown(mousePosition);

        if (IsMousePositionValid(mousePosition) && IsMousePositionValid(mousePositionPrevious))
        {
            mousePositionDelta = (mousePositionPrevious - mousePosition) * 0.003f;
        }
        else
        {
            mousePositionDelta = DkVec2(0.0f, 0.0f);
        }

        mousePositionPrevious = mousePosition;
    }

    bool IsKeyPressed(KeyCode key)
    {
        DkContext& deako = Deako::GetContext();
        auto state = glfwGetKey(deako.window->glfwWindow, static_cast<DkS32>(key));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool IsMousePressed(MouseCode button)
    {
        DkContext& deako = Deako::GetContext();
        auto state = glfwGetMouseButton(deako.window->glfwWindow, static_cast<DkS32>(button));
        return state == GLFW_PRESS;
    }

    bool IsMouseStationary()
    {
        DkInput& input = Deako::GetInput();
        // threshold for occasional small movement tolerance
        const DkF32 stationaryThreshold = 2.0f;
        DkF32 stationaryThresholdSqr = stationaryThreshold * stationaryThreshold;
        return glm::length2(input.mousePositionDelta) <= stationaryThresholdSqr;
    }

    bool IsMousePositionValid(const DkVec2& position)
    {
        DkVec2 windowSize = Deako::GetWindow().GetScaledSize();
        bool validX = (position.x <= windowSize.x) && (position.x >= 0);
        bool validY = (position.y <= windowSize.y) && (position.y >= 0);
        return validX && validY;
    }

    DkVec2 ScaleMousePosition()
    {
        // DkWindow& window = Deako::GetWindow();
        // glfwGetCursorPos(window.glfwWindow, &mousePosition.x, &mousePosition.y);

        // DkVec2 dpiScale = window.GetDPI();
        // mousePosition.x *= dpiScale.x;
        // mousePosition.x *= dpiScale.y;
        return DkVec2();
    }

    void BlockEvents(bool blocked)
    {
        DkInput& input = Deako::GetInput();
        input.eventsBlocked = blocked;
    }

    bool AreEventsBlocked()
    {
        DkInput& input = Deako::GetInput();
        return input.eventsBlocked;
    }


}
