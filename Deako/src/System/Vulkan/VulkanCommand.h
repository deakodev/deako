#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanCommandPool
    {
    public:
        static void Create();
        static void CreateBuffers();
        static void CleanUp();

        static VkCommandBuffer Record(uint32_t currentFrame, uint32_t imageIndex);

        static VkCommandBuffer BeginSingleTimeCommands();
        static void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    private:
        static std::vector<VkCommandBuffer> s_CommandBuffers;
    };

}
