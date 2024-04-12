#pragma once

#include "Base.h"
#include "Deak/Core/KeyCodes.h"
#include "Deak/Core/MouseCodes.h"

namespace Deak {

    class Input
    {
    public:
        virtual ~Input() = default;

        inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

        inline static bool IsMouseButtonPressed(MouseCode button) { return s_Instance->IsMouseButtonPressedImpl(button); }
        inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
        inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
        inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

        // Purpose for the below virtual functions is so that the implentation of the above can be plateform specific
    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;

        virtual bool IsMouseButtonPressedImpl(MouseCode button) = 0;
        virtual std::pair<float, float> GetMousePositionImpl() = 0;
        virtual float GetMouseXImpl() = 0;
        virtual float GetMouseYImpl() = 0;

    private:
        static Scope<Input> s_Instance;
    };

}
