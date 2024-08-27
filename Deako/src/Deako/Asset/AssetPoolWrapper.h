#pragma once

#include "Asset.h"
#include "AssetPoolBase.h"
#include "GLTFImporter.h"

#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"

namespace Deako {


    class AssetPool
    {
    public:
        static void AddAsset(Ref<Asset> asset, const AssetMetadata& metadata)
        {
            Project::GetActive()->GetAssetPool()->AddAsset(asset, metadata);
        }

        template <typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            Ref<Asset> asset = Project::GetActive()->GetAssetPool()->GetAsset(handle);
            return std::static_pointer_cast<T>(asset);
        }

        template <typename T>
        static void DestroyAsset(AssetHandle handle)
        {
            Project::GetActive()->GetAssetPool()->DestroyAsset(handle);
        }

        template <typename T>
        static Ref<T> ImportAsset(const std::string& path)
        {
            AssetType type = AssetTypeFromTypeIndex(std::type_index(typeid(T)));

            std::filesystem::path fullPath = Project::GetAssetDirectory() / path;

            Ref<Asset> asset = Project::GetActive()->GetAssetPool()->ImportAsset(type, fullPath);
            return std::static_pointer_cast<T>(asset);
        }

        static Ref<Model> ImportGLTF(const std::filesystem::path& path)
        {
            std::filesystem::path fullPath = Project::GetAssetDirectory() / path;
            return GLTFImporter::ImportGLTF(fullPath);
        }

    };

}
