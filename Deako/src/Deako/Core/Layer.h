#pragma once

#include "Deako/Core/Base.h"
#include "Deako/Events/Event.h"

#include <imgui.h>

namespace Deako {

    class Layer
    {
    public:
        Layer(std::string_view name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate() {}
        virtual void OnImGuiRender() {}
        virtual void OnImGuiRender(ImTextureID textureID) {}
        virtual void OnEvent(Event& event) {}

        const std::string_view GetName() const { return m_DebugName; }

    protected:
        std::string_view m_DebugName;
    };

}

