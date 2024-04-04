#pragma once

#include "Base.h"
#include "Deak/Events/Event.h"
#include "Timestep.h"

namespace Deak {

    class Layer
    {
    public:
        Layer(std::string_view name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(Timestep timestep) {}
        virtual void OnImGuiRender() {};
        virtual void OnEvent(Event& event) {}

        inline const std::string_view GetName() const { return m_DebugName; }

    protected:
        std::string_view m_DebugName;
    };

}

