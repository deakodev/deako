#pragma once

#include "Deako/Event/Event.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace Deako {

    class Layer
    {
    public:
        Layer(const char* name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate() {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& event) {}

        const char* GetName() const { return m_DebugName; }

    protected:
        const char* m_DebugName;
    };

}

