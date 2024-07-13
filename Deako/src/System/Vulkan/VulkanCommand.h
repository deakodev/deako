#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanCommandPool
    {
    public:
        static void Create();
        static void CreateBuffers(std::vector<VkCommandBuffer>& commandBuffers);
        static void CleanUp();

        static VkCommandBuffer Record(uint32_t currentFrame, uint32_t imageIndex);

        static VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool);
        static void EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer);


        // TODO: temp
        static VkCommandBuffer GetViewportCommandBuffer(uint32_t currentFrame) { return s_ViewportCommandBuffers[currentFrame]; }

    private:
        static std::vector<VkCommandBuffer> s_CommandBuffers;
        static std::vector<VkCommandBuffer> s_ViewportCommandBuffers;
    };

}
