#include "VulkanDepth.h"
#include "dkpch.h"

#include "VulkanBuffer.h"
#include "VulkanTexture.h"

namespace Deako {

    Ref<VulkanResources> Depth::s_VR = VulkanBase::GetResources();

    void Depth::CreateAttachment()
    {
        VkFormat depthFormat = FindFormat();

        SetImageInfo(s_VR->imageExtent.width, s_VR->imageExtent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        SetImageViewInfo(depthFormat);

        Texture::TransitionImageLayout(s_VR->viewportCommandPool, s_VR->depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    void Depth::CleanUp()
    {
        vkDestroyImageView(s_VR->device, s_VR->depthImageView, nullptr);
        vkDestroyImage(s_VR->device, s_VR->depthImage, nullptr);
        vkFreeMemory(s_VR->device, s_VR->depthImageMemory, nullptr);
    }

    void Depth::SetImageInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkImageCreateInfo imageInfo = VulkanInitializers::ImageCreateInfo();
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

        VkResult result = vkCreateImage(s_VR->device, &imageInfo, nullptr, &s_VR->depthImage);
        DK_CORE_ASSERT(!result);

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(s_VR->device, s_VR->depthImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo = VulkanInitializers::MemoryAllocateInfo();
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Buffer::FindMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(s_VR->device, &allocInfo, nullptr, &s_VR->depthImageMemory);
        DK_CORE_ASSERT(!result);

        vkBindImageMemory(s_VR->device, s_VR->depthImage, s_VR->depthImageMemory, 0);
    }

    void Depth::SetImageViewInfo(VkFormat format)
    {
        VkImageViewCreateInfo createInfo = VulkanInitializers::ImageViewCreateInfo();
        createInfo.image = s_VR->depthImage;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        VkResult result = vkCreateImageView(s_VR->device, &createInfo, nullptr, &s_VR->depthImageView);
        DK_CORE_ASSERT(!result);
    }

    VkFormat Depth::FindFormat()
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkFormat Depth::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(s_VR->physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }
        DK_CORE_ASSERT(false);
        return VK_FORMAT_UNDEFINED;
    }

    bool Depth::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

}
