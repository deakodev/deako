#include "TextureHandler.h"
#include "dkpch.h"

#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Pool/EditorAssetPool.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"

#include "Deako/Renderer/Vulkan/VulkanUtils.h"
#include "Deako/Renderer/RendererTypes.h"
#include "System/MacOS/MacUtils.h" 

#include <gli/gli.hpp>
#include <stb_image.h>

namespace Deako {

    void TextureHandler::Init()
    {
        // empty texture2d
        AssetHandle texture2DHandle;
        AssetMetadata texture2DMetadata;
        texture2DMetadata.assetType = AssetType::Texture2D;
        texture2DMetadata.assetPath = "Deako/assets/empty/emptyTexture.ktx";
        texture2DMetadata.assetName = "Empty";

        s_EmptyTexture2D = std::static_pointer_cast<Texture2D>(AssetManager::ImportAsset(texture2DHandle, texture2DMetadata));

        // empty texture cube map
        AssetHandle textureCubeMapHandle;
        AssetMetadata textureCubeMapMetadata;
        textureCubeMapMetadata.assetType = AssetType::TextureCubeMap;
        textureCubeMapMetadata.assetPath = "Deako/assets/empty/emptyCubeMap.ktx";
        textureCubeMapMetadata.assetName = "Empty";

        s_EmptyTextureCubeMap = std::static_pointer_cast<TextureCubeMap>(AssetManager::ImportAsset(textureCubeMapHandle, textureCubeMapMetadata));
    }

    void TextureHandler::CleanUp()
    {
    }

    void TextureHandler::OpenTexture2D()
    {
        std::filesystem::path texturePath = MacUtils::File::Open("ktx", "Import Texture2D");
        ImportTexture2D(texturePath);
    }

    Ref<Texture2D> TextureHandler::ImportTexture2D(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Texture2D;
        metadata.assetPath = path;

        Ref<Texture2D> texture = ImportTexture2D(handle, metadata);

        if (texture)
        {
            texture->m_Handle = handle;
            std::string assetName = metadata.assetPath.filename().string();
            assetName[0] = std::toupper(assetName[0]);
            metadata.assetName = assetName;
        }

        return texture;
    }

    Ref<Texture2D> TextureHandler::ImportTexture2D(AssetHandle handle, AssetMetadata& metadata)
    {
        DK_CORE_INFO("Importing Texture2D <{0}>", metadata.assetPath.filename().string());

        uint32_t width{ 0 }, height{ 0 }, mipLevels{ 1 };
        std::vector<uint32_t> mipLevelWidths{ 1 }, mipLevelHeights{ 1 }, mipLevelOffsets{ 1 };
        Buffer buffer;

        if (metadata.assetPath.extension() == ".ktx")
        {
            gli::texture2d texture(gli::load(metadata.assetPath.string()));
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
            uint8_t* data = stbi_load(metadata.assetPath.c_str(), reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height), &channels, STBI_rgb_alpha);
            DK_CORE_ASSERT(data, "Unable to load texture from file!");
            // TODO: think about mips
            mipLevelWidths[0] = width;
            mipLevelHeights[0] = height;
            mipLevelOffsets[0] = 0;

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
        texture->m_Handle = handle;

        if (metadata.assetName != "Empty")
        {
            ProjectAssetPool::Get()->AddAssetToPool(texture, metadata);
        }
        else
        {
            EditorAssetPool::Get()->AddAssetToPool(texture);
        }

        return texture;
    }

    void TextureHandler::OpenTextureCubeMap()
    {
        std::filesystem::path texturePath = MacUtils::File::Open("ktx", "Import TextureCubeMap");
        ImportTextureCubeMap(texturePath);
    }

    Ref<TextureCubeMap> TextureHandler::ImportTextureCubeMap(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::TextureCubeMap;
        metadata.assetPath = path;

        Ref<TextureCubeMap> textureCube = ImportTextureCubeMap(handle, metadata);

        if (textureCube)
        {
            textureCube->m_Handle = handle;
            std::string assetName = metadata.assetPath.filename().string();
            assetName[0] = std::toupper(assetName[0]);
            metadata.assetName = assetName;
        }

        return textureCube;
    }

    Ref<TextureCubeMap> TextureHandler::ImportTextureCubeMap(AssetHandle handle, AssetMetadata& metadata)
    {
        DK_CORE_INFO("Importing TextureCube <{0}>", metadata.assetPath.filename().string());

        uint32_t width{ 0 }, height{ 0 }, mipLevels{ 1 };
        std::vector<uint32_t> mipLevelWidths, mipLevelHeights, mipLevelOffsets;
        Buffer buffer;

        gli::texture_cube textureCube(gli::load(metadata.assetPath.string()));
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
        texture->m_Handle = handle;

        if (metadata.assetName != "Empty")
        {
            ProjectAssetPool::Get()->AddAssetToPool(texture, metadata);
        }
        else
        {
            EditorAssetPool::Get()->AddAssetToPool(texture);
        }

        return texture;
    }




}
