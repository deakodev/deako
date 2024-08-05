#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

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

    struct MultisampleTarget
    {
        AllocatedImage                            color;
        AllocatedImage                            depth;
    };

}
