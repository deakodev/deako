#include "VulkanDepth.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

namespace Deako {

    void DepthAttachment::Create()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        vr->depthAttachment = CreateScope<DepthAttachment>();
    }

    DepthAttachment::DepthAttachment()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkFormat depthFormat = FindDepthFormat();

        this->SetImageInfo(vr->imageExtent.width, vr->imageExtent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        this->SetImageViewInfo(depthFormat);

        Texture::TransitionImageLayout(vr->viewportCommandPool, m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    DepthAttachment::~DepthAttachment() {
        VulkanResources* vr = VulkanBase::GetResources();
        vkDestroyImageView(vr->device, m_DepthImageView, nullptr);
        vkDestroyImage(vr->device, m_DepthImage, nullptr);
        vkFreeMemory(vr->device, m_DepthImageMemory, nullptr);
    }

    void DepthAttachment::SetImageInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        VkResult result = vkCreateImage(vr->device, &imageInfo, nullptr, &m_DepthImage);
        DK_CORE_ASSERT(!result);

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vr->device, m_DepthImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Buffer::FindMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(vr->device, &allocInfo, nullptr, &m_DepthImageMemory);
        DK_CORE_ASSERT(!result);

        vkBindImageMemory(vr->device, m_DepthImage, m_DepthImageMemory, 0);
    }

    void DepthAttachment::SetImageViewInfo(VkFormat format)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_DepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        VkResult result = vkCreateImageView(vr->device, &viewInfo, nullptr, &m_DepthImageView);
        DK_CORE_ASSERT(!result);
    }

    VkFormat DepthAttachment::FindDepthFormat()
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkFormat DepthAttachment::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(vr->physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }
        DK_CORE_ASSERT(false);
    }

    bool DepthAttachment::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

}
