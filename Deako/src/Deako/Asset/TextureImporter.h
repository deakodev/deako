#pragma once

#include "Asset.h"

#include "Deako/Renderer/Vulkan/VulkanTexture.h"

namespace Deako {

    class TextureImporter
    {
    public:
        static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
        static Ref<TextureCubeMap> ImportTextureCubeMap(AssetHandle handle, const AssetMetadata& metadata);

    };

}
