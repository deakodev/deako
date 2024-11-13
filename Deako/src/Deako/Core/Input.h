#pragma once

#include "Deako/Event/KeyCodes.h"
#include "Deako/Event/MouseCodes.h"

#include <GLFW/glfw3.h>


namespace Deako {

    struct DkInput
    {
        DkVec2 mousePosition = { -FLT_MAX, -FLT_MAX };
        DkVec2 mousePositionPrevious = { -FLT_MAX, -FLT_MAX };
        DkVec2 mousePositionDelta = { 0.0f, 0.0f };
        bool eventsBlocked{ false };

        void OnUpdate();
        void UpdateMouse();
    };

    bool IsKeyPressed(KeyCode key);
    bool IsMousePressed(MouseCode button);
    bool IsMouseStationary();

    bool IsMousePositionValid(const DkVec2& position);
    DkVec2 ScaleMousePosition();

    void BlockEvents(bool blocked);
    bool AreEventsBlocked();

}
