#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanPicker
    {
    public:
        static void Init();
        static void CleanUp();

        static void Draw(VkCommandBuffer commandBuffer, DkU32 imageIndex);

        static const DkVec4& GetPixelColor();
    };

}
