#pragma once

#include "Deako/Core/Layer.h"
#include "Deako/Event/KeyEvent.h"
#include "Deako/Event/MouseEvent.h"

// #include "Deako/Renderer/Vulkan/VulkanBase.h"

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

        void Begin();
        void End(VkCommandBuffer commandBuffer);

        void BlockEvents(bool block) { m_BlockEvents = block; }

        static void SetStyles();

    private:
        static void SetDarkThemeColors();

    private:
        bool m_BlockEvents = true;
        float m_Time = 0.0f;

        // static Ref<VulkanResources> s_VR;
        // static Ref<VulkanSettings> s_VS;
    };

}
