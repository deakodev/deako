#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class Texture
    {
    public:
        Texture(const std::string& path, VkFormat format, VkImageUsageFlags usage, VkImageLayout imageLayout);

        VkImage& GetImage() { return m_Image; }
        VkImageView& GetImageView() { return m_ImageView; }
        VkDeviceMemory& GetMemory() { return m_ImageMemory; }

        void SetImageInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void SetImageViewInfo(VkFormat format);
        void CopyStaging(VkBuffer stagingBuffer, VkImage receivingImage, uint32_t width, uint32_t height);
        static void TransitionImageLayout(VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    private:
        VkImage m_Image{ VK_NULL_HANDLE };
        VkImageView m_ImageView{ VK_NULL_HANDLE };
        VkDeviceMemory m_ImageMemory{ VK_NULL_HANDLE };

        static  Ref<VulkanResources> s_VR;
    };

    class Texture2D : public Texture
    {
    public:
        Texture2D(const std::string& path, VkFormat format, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            :Texture(path, format, usage, imageLayout)
        {}

    };

    class TexturePool
    {
    public:
        static void CreateSamplers();
        // static void CreateTextures();
        static void CleanUp();

        static VkSampler& GetTextureSampler() { return s_TextureSampler; }
        static std::vector<Ref<Texture2D>>& GetTexture2Ds() { return s_Textures; }

    private:
        static VkSampler s_TextureSampler;
        static std::vector<Ref<Texture2D>> s_Textures;

        static  Ref<VulkanResources> s_VR;
    };

}
