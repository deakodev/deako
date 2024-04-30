#pragma once

#include "Base.h"
#include "Deak/Core/KeyCodes.h"
#include "Deak/Core/MouseCodes.h"

namespace Deak {

    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode keycode);

        static bool IsMouseButtonPressed(MouseCode button);
        static std::pair<float, float> GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };

}
