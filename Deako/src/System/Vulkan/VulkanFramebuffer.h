#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanFramebufferPool
    {
    public:
        static void Create();
        static void CleanUp();

        static const std::vector<VkFramebuffer>& GetFramebuffers() { return s_Framebuffers; }

    private:
        static std::vector<VkFramebuffer> s_Framebuffers;
    };

}
