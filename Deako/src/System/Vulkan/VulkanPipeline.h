#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanPipeline
    {
    public:
        static void Create();
        static void CleanUp();
    };

}
