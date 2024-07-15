#include "VulkanTexture.h"
#include "dkpch.h"

#include "VulkanBuffer.h"
#include "VulkanCommand.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Deako {

    VkSampler TexturePool::s_TextureSampler;
    Ref<Texture> TexturePool::s_ViewportTexture;
    Ref<VulkanResources> TexturePool::s_VR = VulkanBase::GetResources();
    Ref<VulkanResources> Texture::s_VR = VulkanBase::GetResources();

    Texture::Texture()
    {
        int texWidth, texHeight, texChannels;

        stbi_uc* pixels = stbi_load("Deako-Editor/assets/textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        DK_CORE_ASSERT(pixels);

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        Buffer stagingBuffer{};
        stagingBuffer.SetInfo(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.Map();
        stagingBuffer.CopyTo(pixels, (size_t)imageSize);
        stagingBuffer.Unmap();

        stbi_image_free(pixels);

        this->SetImageInfo(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        TransitionImageLayout(s_VR->viewportCommandPool, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyStaging(stagingBuffer.GetBuffer(), m_Image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(s_VR->viewportCommandPool, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(s_VR->device, stagingBuffer.GetBuffer(), nullptr);
        vkFreeMemory(s_VR->device, stagingBuffer.GetMemory(), nullptr);

        this->SetImageViewInfo(VK_FORMAT_R8G8B8A8_SRGB);
    }

    void Texture::SetImageInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
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

        VkResult result = vkCreateImage(s_VR->device, &imageInfo, nullptr, &m_Image);
        DK_CORE_ASSERT(!result);

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(s_VR->device, m_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = VulkanInitializers::MemoryAllocateInfo();
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Buffer::FindMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(s_VR->device, &allocInfo, nullptr, &m_ImageMemory);
        DK_CORE_ASSERT(!result);

        vkBindImageMemory(s_VR->device, m_Image, m_ImageMemory, 0);
    }

    void Texture::SetImageViewInfo(VkFormat format)
    {
        VkImageViewCreateInfo createInfo = VulkanInitializers::ImageViewCreateInfo();
        createInfo.image = m_Image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        VkResult result = vkCreateImageView(s_VR->device, &createInfo, nullptr, &m_ImageView);
        DK_CORE_ASSERT(!result);
    }

    void Texture::CopyStaging(VkBuffer stagingBuffer, VkImage receivingImage, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = CommandPool::BeginSingleTimeCommands(s_VR->commandPool);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, receivingImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        CommandPool::EndSingleTimeCommands(s_VR->commandPool, commandBuffer);
    }

    void Texture::TransitionImageLayout(VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = CommandPool::BeginSingleTimeCommands(commandPool);

        VkImageMemoryBarrier barrier = VulkanInitializers::ImageMemoryBarrier();
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (Depth::HasStencilComponent(format))
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else
        {
            DK_CORE_ASSERT(false);
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        CommandPool::EndSingleTimeCommands(commandPool, commandBuffer);
    }

    void TexturePool::CreateSamplers()
    {
        VkSamplerCreateInfo samplerInfo = VulkanInitializers::SamplerCreateInfo();
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(s_VR->physicalDevice, &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkResult result = vkCreateSampler(s_VR->device, &samplerInfo, nullptr, &s_TextureSampler);
        DK_CORE_ASSERT(!result);
    }

    void TexturePool::CreateTextures()
    {
        s_ViewportTexture = CreateRef<Texture>();
    }

    void TexturePool::CleanUp()
    {
        vkDestroySampler(s_VR->device, s_TextureSampler, nullptr);

        vkDestroyImageView(s_VR->device, s_ViewportTexture->GetImageView(), nullptr);
        vkDestroyImage(s_VR->device, s_ViewportTexture->GetImage(), nullptr);
        vkFreeMemory(s_VR->device, s_ViewportTexture->GetMemory(), nullptr);
    }

}
