#pragma once

#include "Asset.h"

#include "Deako/Renderer/Vulkan/VulkanTexture.h"

namespace Deako {

    class TextureImporter
    {
    public:
        static Ref<Texture2D> ImportTexture2D(const AssetMetadata& metadata);
        static Ref<Texture2D> ImportTexture2DViaPath(const std::filesystem::path& path);
        static Ref<TextureCubeMap> ImportTextureCubeMap(const AssetMetadata& metadata);

    };

}
