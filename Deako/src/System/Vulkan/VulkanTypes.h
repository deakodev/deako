#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    struct FrameData
    {
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        // DescriptorAllocatorGrowable descriptors;

        VkSemaphore renderSemaphore;
        VkSemaphore presentSemaphore;
        VkFence waitFence;
    };

    struct SwapchainDetails
    {
        VkSurfaceCapabilitiesKHR                  capabilities;
        std::vector<VkSurfaceFormatKHR>           formats;
        std::vector<VkPresentModeKHR>             presentModes;
    };

    struct AllocatedBuffer
    {
        VkBuffer                                  buffer;
        VkDeviceMemory                            memory;
        VkMemoryRequirements                      memReqs;
        void* /*                               */ mapped;
    };

    struct UniformBuffer
    {
        AllocatedBuffer                           buffer;
        VkDescriptorBufferInfo                    descriptor;
    };

    struct AllocatedImage
    {
        VkImage                                   image;
        VkImageView                               view;
        VkDeviceMemory                            memory;
        VkExtent3D                                extent;
        VkFormat                                  format;
    };

}
