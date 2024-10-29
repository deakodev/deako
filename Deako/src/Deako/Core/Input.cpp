#include "Input.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

namespace Deako {

    void Input::Init()
    {
        m_Window = GetApplication().GetWindow().GetNativeWindow();
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
        double mouseX, mouseY;
        glfwGetCursorPos(m_Window.get(), &mouseX, &mouseY);

        float xscale, yscale;
        glfwGetWindowContentScale(m_Window.get(), &xscale, &yscale);

        int width, height;
        glfwGetWindowSize(m_Window.get(), &width, &height);

        mouseX *= xscale;
        mouseY *= yscale;
        width *= yscale;
        height *= yscale;

        if (mouseX >= 0 && mouseY >= 0 && mouseX <= width && mouseY <= height)
            return { mouseX, mouseY }; // mouse is inside window
        else
            return { 0, 0 }; // mouse is outside window
    }





}
