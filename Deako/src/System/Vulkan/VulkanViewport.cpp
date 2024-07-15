#include "VulkanViewport.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

#include "VulkanFramebuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanCommand.h"

namespace Deako {

    Ref<VulkanResources> Viewport::s_VR = VulkanBase::GetResources();

    void Viewport::Create()
    {
        {   // Images
            s_VR->viewportImages.resize(s_VR->swapChainImages.size());
            s_VR->viewportImageMemory.resize(s_VR->swapChainImages.size());

            for (uint32_t i = 0; i < s_VR->viewportImages.size(); i++)
            {
                VkImageCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                createInfo.imageType = VK_IMAGE_TYPE_2D;
                createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                createInfo.extent.width = s_VR->imageExtent.width;
                createInfo.extent.height = s_VR->imageExtent.height;
                createInfo.extent.depth = 1;
                createInfo.arrayLayers = 1;
                createInfo.mipLevels = 1;
                createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                createInfo.tiling = VK_IMAGE_TILING_LINEAR;
                createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

                vkCreateImage(s_VR->device, &createInfo, nullptr, &s_VR->viewportImages[i]);

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(s_VR->device, s_VR->viewportImages[i], &memRequirements);

                VkMemoryAllocateInfo memAllocInfo{};
                memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                memAllocInfo.allocationSize = memRequirements.size;
                memAllocInfo.memoryTypeIndex = Buffer::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                vkAllocateMemory(s_VR->device, &memAllocInfo, nullptr, &s_VR->viewportImageMemory[i]);
                vkBindImageMemory(s_VR->device, s_VR->viewportImages[i], s_VR->viewportImageMemory[i], 0);

                InsertImageMemoryBarrier(
                    s_VR->viewportCommandPool,
                    s_VR->viewportImages[i],
                    VK_ACCESS_TRANSFER_READ_BIT,
                    VK_ACCESS_MEMORY_READ_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
                );
            }
        }

        {   // Image views
            s_VR->viewportImageViews.resize(s_VR->swapChainImages.size());

            for (uint32_t i = 0; i < s_VR->viewportImageViews.size(); i++)
            {
                VkImageViewCreateInfo createInfo = VulkanInitializers::ImageViewCreateInfo();
                createInfo.image = s_VR->viewportImages[i];
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseMipLevel = 0;
                createInfo.subresourceRange.levelCount = 1;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.layerCount = 1;

                VkResult result = vkCreateImageView(s_VR->device, &createInfo, nullptr, &s_VR->viewportImageViews[i]);
                DK_CORE_ASSERT(result == VK_SUCCESS);
            }
        }
    }

    void Viewport::InsertImageMemoryBarrier(VkCommandPool commandPool, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
    {
        VkCommandBuffer commandBuffer = CommandPool::BeginSingleTimeCommands(commandPool);

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        CommandPool::EndSingleTimeCommands(commandPool, commandBuffer);
    }

    void Viewport::CleanUp()
    {
        for (auto image : s_VR->viewportImages)
            vkDestroyImage(s_VR->device, image, nullptr);
        for (auto imageView : s_VR->viewportImageViews)
            vkDestroyImageView(s_VR->device, imageView, nullptr);
        for (auto imageMemory : s_VR->viewportImageMemory)
            vkFreeMemory(s_VR->device, imageMemory, nullptr);
    }

}
