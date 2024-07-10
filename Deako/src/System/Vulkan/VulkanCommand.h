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

        static VkCommandPool& GetPool() { return s_CommandPool; }

        static VkCommandBuffer BeginSingleTimeCommands();
        static void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    private:
        static VkDevice s_Device;
        static VkCommandPool s_CommandPool;
        static std::vector<VkCommandBuffer> s_CommandBuffers;
    };

}
