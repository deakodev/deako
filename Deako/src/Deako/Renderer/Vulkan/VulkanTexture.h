#pragma once

#include "VulkanTypes.h"
#include "Deako/Asset/Asset.h"

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

    struct TextureDetails
    {
        VkFormat format;
        VkImageUsageFlags usage;
        VkImageLayout layout;

        uint32_t width{ 0 };
        uint32_t height{ 0 };
        uint32_t mipLevels{ 1 };

        std::vector<uint32_t> mipLevelWidths;
        std::vector<uint32_t> mipLevelHeights;
        std::vector<uint32_t> mipLevelOffsets;
    };

    struct Texture : public Asset
    {
        TextureDetails details;

        AllocatedImage image;
        VkSampler sampler;
        VkDescriptorImageInfo descriptor;
        uint32_t mipLevels; // TODO: remove

        Texture() {}
        Texture(TextureDetails details);

        void UpdateDescriptor();
        void Destroy();
    };

    struct Texture2D : public Texture
    {
        Texture2D() {}
        Texture2D(const TextureDetails& details, Buffer buffer = Buffer());

        void LoadFromGLTFImage(tinygltf::Image& gltfimage, std::filesystem::path path, TextureSampler textureSampler);

        static AssetType GetStaticType() { return AssetType::Texture2D; }
        virtual AssetType GetType() const override { return GetStaticType(); }
    };

    struct TextureCubeMap : public Texture
    {
        enum Target { NONE = 0, IRRADIANCE = 1, PREFILTERED = 2 } target;

        TextureCubeMap(const TextureDetails& details, Buffer buffer = Buffer());

        TextureCubeMap(Target target) : target(target) {}

        void LoadFromFile(std::filesystem::path path, VkFormat format, VkImageUsageFlags imageUsageFlags =
            VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        void GenerateCubeMap();

        static AssetType GetStaticType() { return AssetType::TextureCubeMap; }
        virtual AssetType GetType() const override { return GetStaticType(); }

    };

    void LoadEnvironment(std::filesystem::path path);

}
