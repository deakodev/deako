#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanRenderPass
    {
    public:
        static void Create();
        static void CleanUp();
    };

}
