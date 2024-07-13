#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class DepthAttachment
    {
    public:
        DepthAttachment();
        ~DepthAttachment();

        VkImage& GetImage() { return m_DepthImage; }
        VkImageView& GetImageView() { return m_DepthImageView; }
        VkDeviceMemory& GetMemory() { return m_DepthImageMemory; }

        void SetImageInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void SetImageViewInfo(VkFormat format);

        static VkFormat FindDepthFormat();
        static bool HasStencilComponent(VkFormat format);

        static void Create();

    private:
        static VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    private:
        VkImage m_DepthImage{ VK_NULL_HANDLE };
        VkImageView m_DepthImageView{ VK_NULL_HANDLE };
        VkDeviceMemory m_DepthImageMemory{ VK_NULL_HANDLE };
    };

}
