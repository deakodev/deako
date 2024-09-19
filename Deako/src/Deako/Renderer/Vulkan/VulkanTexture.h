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

        VkImageView GetImageView() { return image.view; }

        void UpdateDescriptor();
        virtual void Destroy() override;
    };

    struct Texture2D : public Texture
    {
        Texture2D() {}
        Texture2D(const TextureDetails& details, Buffer& buffer);

        void GenerateFromGLTF(tinygltf::Image& gltfimage, Buffer& textureData, TextureSampler textureSampler);

        static AssetType GetStaticType() { return AssetType::Texture2D; }
        virtual AssetType GetType() const override { return GetStaticType(); }
    };

    struct TextureCubeMap : public Texture
    {
        enum Target { NONE = 0, IRRADIANCE = 1, PREFILTERED = 2 } target;

        TextureCubeMap(const TextureDetails& details, Buffer& buffer);

        TextureCubeMap(Target target) : target(target) {}

        void GenerateCubeMap();

        static AssetType GetStaticType() { return AssetType::TextureCubeMap; }
        virtual AssetType GetType() const override { return GetStaticType(); }

    };

    void LoadEnvironment(std::filesystem::path path);

}
