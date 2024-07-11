#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class Texture
    {
    public:
        Texture();

        VkImage& GetImage() { return m_Image; }
        VkImageView& GetImageView() { return m_ImageView; }
        VkDeviceMemory& GetMemory() { return m_ImageMemory; }

        void SetImageInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void SetImageViewInfo(VkFormat format);
        void CopyStaging(VkBuffer stagingBuffer, VkImage receivingImage, uint32_t width, uint32_t height);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    private:
        VkImage m_Image{ VK_NULL_HANDLE };
        VkImageView m_ImageView{ VK_NULL_HANDLE };
        VkDeviceMemory m_ImageMemory{ VK_NULL_HANDLE };
    };

    class TextureSampler
    {
    public:
        TextureSampler();

        VkSampler& GetSampler() { return m_Sampler; }

    private:
        VkSampler m_Sampler{ VK_NULL_HANDLE };
    };

    class VulkanTexturePool
    {
    public:
        static void CreateTextures();
        static void CleanUp();

        static Ref<Texture> GetTexture() { return s_Texture; }
        static Ref<TextureSampler> GetTextureSampler() { return s_TextureSampler; }

    private:
        static Ref<Texture> s_Texture;
        static Ref<TextureSampler> s_TextureSampler;
    };

}
