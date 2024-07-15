#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    struct VulkanResources;

    class Depth
    {
    public:
        static void CreateAttachment();
        static void CleanUp();

        static void SetImageInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        static void SetImageViewInfo(VkFormat format);

        static VkFormat FindFormat();
        static bool HasStencilComponent(VkFormat format);

    private:
        static VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    private:
        static Ref<VulkanResources> s_VR;
    };

}
