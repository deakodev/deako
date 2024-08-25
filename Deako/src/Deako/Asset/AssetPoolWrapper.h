#pragma once

#include "Asset.h"
#include "AssetPoolBase.h"

#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"

namespace Deako {


    class AssetPool
    {
    public:
        template <typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            Ref<Asset> asset = Project::GetActive()->GetAssetPool()->GetAsset(handle);
            return std::static_pointer_cast<T>(asset);
        }

        template <typename T>
        static void DestroyAsset(AssetHandle handle)
        {
            Project::GetActive()->GetEditorAssetPool()->DestroyAsset(handle);
        }

        template <typename T>
        static Ref<T> ImportAsset(const std::string& path)
        {
            AssetType type;

            if constexpr (std::is_same_v<T, Texture2D>)
                type = AssetType::Texture2D;
            else if constexpr (std::is_same_v<T, TextureCubeMap>)
                type = AssetType::TextureCubeMap;
            else if constexpr (std::is_same_v<T, Model>)
                type = AssetType::Model;
            else if constexpr (std::is_same_v<T, Scene>)
                type = AssetType::Scene;

            std::filesystem::path fullPath = Project::GetAssetDirectory() / path;

            Ref<Asset> asset = Project::GetActive()->GetEditorAssetPool()->ImportAsset(type, fullPath);
            return std::static_pointer_cast<T>(asset);
        }

    };

}
