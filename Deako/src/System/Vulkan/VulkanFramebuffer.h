#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanFramebufferPool
    {
    public:
        static void Create();
        static void CleanUp();

        static const VkFramebuffer& GetFramebuffer(uint32_t currentFrame) { return s_Framebuffers[currentFrame]; }
        static const VkFramebuffer& GetViewportFramebuffer(uint32_t currentFrame) { return s_ViewportFramebuffers[currentFrame]; }

    private:
        static std::vector<VkFramebuffer> s_Framebuffers;
        static std::vector<VkFramebuffer> s_ViewportFramebuffers;
    };

}
