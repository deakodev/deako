#include "Input.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

namespace Deako {

    void Input::Init()
    {
        m_Window = Application::Get().GetWindow().GetNativeWindow();
    }

    bool Input::IsKeyPressed(KeyCode keycode)
    {
        auto state = glfwGetKey(m_Window.get(), static_cast<int32_t>(keycode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(MouseCode button)
    {
        auto state = glfwGetMouseButton(m_Window.get(), static_cast<int32_t>(button));
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition()
    {
        double xpos, ypos;
        glfwGetCursorPos(m_Window.get(), &xpos, &ypos);

        return { (float)xpos, (float)ypos };
    }

}
