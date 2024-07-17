#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class CommandPool
    {
    public:
        static void Create();
        static void CreateBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandBuffers);
        static void CleanUp();

        static void RecordViewportCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t imageIndex);
        static void RecordImGuiCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        static VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool);
        static void EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer);

    private:
        static Ref<VulkanResources> s_VR;
        static Ref<VulkanSettings> s_VS;
    };

}
