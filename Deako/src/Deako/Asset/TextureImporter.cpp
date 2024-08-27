#include "TextureImporter.h"
#include "dkpch.h"

#include "Deako/Renderer/RendererTypes.h"
#include "Deako/Renderer/Vulkan/VulkanUtils.h"

#include <gli/gli.hpp>
#include <stb_image.h>

namespace Deako {

    Ref<Texture2D> TextureImporter::ImportTexture2D(const AssetMetadata& metadata)
    {
        DK_CORE_INFO("Importing Texture2D <{0}>", metadata.path.filename().string());

        uint32_t width{ 0 }, height{ 0 }, mipLevels{ 1 };
        std::vector<uint32_t> mipLevelWidths, mipLevelHeights, mipLevelOffsets;
        Buffer buffer;

        if (metadata.path.extension() == ".ktx")
        {
            gli::texture2d texture(gli::load(metadata.path.string()));
            DK_CORE_ASSERT(!texture.empty(), "Unable to load KTX texture from file!");

            width = static_cast<uint32_t>(texture[0].extent().x);
            height = static_cast<uint32_t>(texture[0].extent().y);
            mipLevels = static_cast<uint32_t>(texture.levels());

            mipLevelWidths.resize(mipLevels);
            mipLevelHeights.resize(mipLevels);
            mipLevelOffsets.resize(mipLevels);
            uint32_t currentOffset = 0;
            for (uint32_t i = 0; i < mipLevels; i++)
            {
                mipLevelWidths[i] = static_cast<uint32_t>(texture[i].extent().x);
                mipLevelHeights[i] = static_cast<uint32_t>(texture[i].extent().y);
                mipLevelOffsets[i] = currentOffset;

                currentOffset += static_cast<uint32_t>(texture[i].size());
            }

            buffer = Buffer(texture.data(), texture.size());
        }
        else
        {
            stbi_set_flip_vertically_on_load(1);

            int channels;
            uint8_t* data = stbi_load(metadata.path.c_str(), reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height), &channels, STBI_rgb_alpha);
            DK_CORE_ASSERT(buffer, "Unable to load texture from file!");

            size_t bufferSize = width * height * channels; // TODO: find better way to determine size?
            buffer = Buffer(data, bufferSize);
            stbi_image_free(data);
        }

        TextureDetails details;
        details.width = width;
        details.height = height;
        details.mipLevels = mipLevels;
        details.mipLevelWidths = mipLevelWidths;
        details.mipLevelHeights = mipLevelHeights;
        details.mipLevelOffsets = mipLevelOffsets;
        details.format = VulkanImage::ConvertToVulkanFormat(ImageFormat::DK_R8G8B8A8_UNORM);
        details.usage = VulkanImage::ConvertToVulkanUsage(ImageUsage::DK_SAMPLED_BIT);
        details.layout = VulkanImage::ConvertToVulkanLayout(ImageLayout::DK_SHADER_READ_ONLY_OPTIMAL);

        Ref<Texture2D> texture = CreateRef<Texture2D>(details, buffer);

        return texture;
    }

    Ref<TextureCubeMap> TextureImporter::ImportTextureCubeMap(const AssetMetadata& metadata)
    {
        DK_CORE_INFO("Importing TextureCube <{0}>", metadata.path.filename().string());

        uint32_t width{ 0 }, height{ 0 }, mipLevels{ 1 };
        std::vector<uint32_t> mipLevelWidths, mipLevelHeights, mipLevelOffsets;
        Buffer buffer;

        gli::texture_cube textureCube(gli::load(metadata.path.string()));
        DK_CORE_ASSERT(!textureCube.empty(), "Unable to load texture cube from file!");

        width = static_cast<uint32_t>(textureCube[0].extent().x);
        height = static_cast<uint32_t>(textureCube[0].extent().y);
        mipLevels = static_cast<uint32_t>(textureCube.levels());

        mipLevelWidths.resize(mipLevels * 6);
        mipLevelHeights.resize(mipLevels * 6);
        mipLevelOffsets.resize(mipLevels * 6);
        uint32_t currentOffset = 0;
        for (uint32_t face = 0; face < 6; face++)
        {
            for (uint32_t level = 0; level < mipLevels; level++)
            {
                uint32_t index = face * mipLevels + level;
                mipLevelWidths[index] = static_cast<uint32_t>(textureCube[face][level].extent().x);
                mipLevelHeights[index] = static_cast<uint32_t>(textureCube[face][level].extent().y);
                mipLevelOffsets[index] = currentOffset;

                currentOffset += static_cast<uint32_t>(textureCube[face][level].size());
            }
        }

        buffer = Buffer(textureCube.data(), textureCube.size());

        TextureDetails details;
        details.width = width;
        details.height = height;
        details.mipLevels = mipLevels;
        details.mipLevelWidths = mipLevelWidths;
        details.mipLevelHeights = mipLevelHeights;
        details.mipLevelOffsets = mipLevelOffsets;
        details.format = VulkanImage::ConvertToVulkanFormat(ImageFormat::DK_R16G16B16A16_SFLOAT);
        details.usage = VulkanImage::ConvertToVulkanUsage(ImageUsage::DK_SAMPLED_BIT);
        details.layout = VulkanImage::ConvertToVulkanLayout(ImageLayout::DK_SHADER_READ_ONLY_OPTIMAL);

        Ref<TextureCubeMap> texture = CreateRef<TextureCubeMap>(details, buffer);

        return texture;
    }


}
