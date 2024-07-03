#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanRenderPass
    {
    public:
        static void Create();
        static void CleanUp();

        static VkRenderPass GetRenderPass() { return s_RenderPass; }

    private:
        static VkRenderPass s_RenderPass;
    };

}
