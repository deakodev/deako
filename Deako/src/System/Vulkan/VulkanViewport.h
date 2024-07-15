#pragma once

#include "VulkanBase.h"

namespace Deako {

    class Viewport
    {
    public:
        static void Create();
        static void CleanUp();

    private:
        static void InsertImageMemoryBarrier(VkCommandPool commandPool, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

    private:
        static Ref<VulkanResources> s_VR;
    };

}
