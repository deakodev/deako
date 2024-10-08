#pragma once

#include "Deako/Renderer/Vulkan/VulkanTexture.h"

namespace Deako {

    class TextureHandler
    {
    public:
        static void Init();
        static void CleanUp();

        static void OpenTexture2D();
        static void OpenTextureCubeMap();

        static Ref<Texture2D> ImportTexture2D(const std::filesystem::path& path);
        static Ref<Texture2D> ImportTexture2D(AssetHandle handle, AssetMetadata& metadata);
        static Ref<TextureCubeMap> ImportTextureCubeMap(const std::filesystem::path& path);
        static Ref<TextureCubeMap> ImportTextureCubeMap(AssetHandle handle, AssetMetadata& metadata);

        static Ref<Texture2D> GetEmptyTexture2D() { return s_EmptyTexture2D; }
        static Ref<TextureCubeMap> GetEmptyTextureCubeMap() { return s_EmptyTextureCubeMap; }

    private:
        inline static Ref<Texture2D> s_EmptyTexture2D;
        inline static Ref<TextureCubeMap> s_EmptyTextureCubeMap;
    };



}
