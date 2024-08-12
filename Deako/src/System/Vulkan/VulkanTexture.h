#pragma once

#include "VulkanTypes.h"

#include <vulkan/vulkan.h>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

namespace Deako {

    struct TextureSampler
    {
        VkFilter magFilter;
        VkFilter minFilter;
        VkSamplerAddressMode addressModeU;
        VkSamplerAddressMode addressModeV;
        VkSamplerAddressMode addressModeW;

        void SetFilterModes(int32_t min, int32_t mag);
        void SetWrapModes(int32_t wrapS, int32_t wrapT);
    };

    class Texture
    {
    public:
        void UpdateDescriptor();
        void Destroy();

        AllocatedImage& GetImage() { return m_Image; }
        VkSampler& GetSampler() { return m_Sampler; }
        VkDescriptorImageInfo& GetDescriptor() { return m_Descriptor; }

    protected:
        AllocatedImage m_Image;
        VkSampler m_Sampler;
        VkImageLayout m_ImageLayout;
        VkDescriptorImageInfo m_Descriptor;
        uint32_t m_MipLevels;
        uint32_t m_LayerCount;
    };

    class Texture2D : public Texture
    {
    public:
        void LoadFromFile(std::filesystem::path path, VkFormat format, VkImageUsageFlags imageUsageFlags =
            VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        void LoadFromGLTFImage(tinygltf::Image& gltfimage, std::filesystem::path path, TextureSampler textureSampler);
    };

    class TextureCubeMap : public Texture
    {
    public:
        enum Target { NONE = 0, IRRADIANCE = 1, PREFILTERED = 2 };

        TextureCubeMap(Target target) : m_Target(target) {}

        void LoadFromFile(std::filesystem::path path, VkFormat format, VkImageUsageFlags imageUsageFlags =
            VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        void GenerateCubeMap();

    private:
        Target m_Target;
    };

    void LoadEnvironment(std::filesystem::path path);

}
