#pragma once

#include "Deak/Core/Layer.h"
#include "Deak/Events/KeyEvent.h"
#include "Deak/Events/MouseEvent.h"
#include "Deak/Events/ApplicationEvent.h"

namespace Deak {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& event) override;

        void Begin();
        void End();

        void BlockEvents(bool block) { m_BlockEvents = block; }

    private:
        bool m_BlockEvents = true;
        float m_Time = 0.0f;
    };

}
