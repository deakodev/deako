#pragma once

#include "Deak/Core/Layer.h"

namespace Deak {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
        void OnEvent(Event& event) override;

        void Begin();
        void End();
    };

}
