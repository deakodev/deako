#pragma once

#include "VulkanResource.h"

#include "Deako/Asset/Asset.h"
#include "Deako/Core/Buffer.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include <vulkan/vulkan.h>

namespace Deako {

    struct TextureSampler
    {
        VkFilter magFilter = VK_FILTER_LINEAR;
        VkFilter minFilter = VK_FILTER_LINEAR;
        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        void SetFilterModes(DkS32 min, DkS32 mag);
        void SetWrapModes(DkS32 wrapS, DkS32 wrapT);
    };

    struct TextureDetails
    {
        VkFormat format;
        VkImageUsageFlags usage;
        VkImageLayout layout;

        DkU32 width{ 0 };
        DkU32 height{ 0 };
        DkU32 mipLevels{ 1 };

        std::vector<DkU32> mipLevelWidths;
        std::vector<DkU32> mipLevelHeights;
        std::vector<DkU32> mipLevelOffsets;
    };

    struct Texture : public Asset
    {
        TextureDetails details;

        AllocatedImage image;
        VkSampler sampler;
        VkDescriptorImageInfo descriptor;
        DkU32 mipLevels; // TODO: remove

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
        enum Target { IRRADIANCE = 0, PREFILTERED = 1 } target;

        TextureCubeMap(const TextureDetails& details, Buffer& buffer);

        TextureCubeMap() {}
        TextureCubeMap(Target target) : target(target) {}

        void GenerateCubeMap();

        static AssetType GetStaticType() { return AssetType::TextureCubeMap; }
        virtual AssetType GetType() const override { return GetStaticType(); }

    };

}
