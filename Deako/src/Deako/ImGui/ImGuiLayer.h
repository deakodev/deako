#pragma once

#include "Deako/Core/Layer.h"
#include "Deako/Events/KeyEvent.h"
#include "Deako/Events/MouseEvent.h"

#include "System/Vulkan/VulkanBase.h"

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
        void End(VkCommandBuffer commandBuffer, VkPipeline pipeline);

        void BlockEvents(bool block) { m_BlockEvents = block; }

        void SetViewportTextureIDs();
        void ResetViewportTextureIDs();
        const std::vector<void*>& GetViewportTextureIDs() { return m_ViewportTextureIDs; }

    private:
        void SetDarkThemeColors();

    private:
        bool m_BlockEvents = true;
        float m_Time = 0.0f;

        VkDescriptorPool m_ImguiPool;
        std::vector<void*> m_ViewportTextureIDs;

        static Ref<VulkanResources> s_VR;
        static Ref<VulkanSettings> s_VS;
    };

}
