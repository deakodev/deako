#pragma once

#include "Base.h"
#include "Deako/Event/KeyCodes.h"
#include "Deako/Event/MouseCodes.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Deako {

    class Input
    {
    public:
        static void Init();

        static bool IsKeyPressed(KeyCode keycode);
        static bool IsMouseButtonPressed(MouseCode button);

        static void BlockEvents(bool block) { s_BlockEvents = block; }

        static glm::vec2 GetMousePosition();

    private:
        inline static Ref<GLFWwindow> m_Window;
        inline static bool s_BlockEvents{ false };
    };

}
