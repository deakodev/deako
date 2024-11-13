#pragma once

#include "Deako/Core/Layer.h"
#include "Deako/Event/KeyEvent.h"
#include "Deako/Event/MouseEvent.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& event) override;

        static void Begin();
        static void End(VkCommandBuffer commandBuffer);

        static void SetStyles();

    private:
        static void SetDarkTheme();
    };

}
