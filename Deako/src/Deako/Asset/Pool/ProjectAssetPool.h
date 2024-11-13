#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Pool/AssetPool.h"

namespace Deako {

    using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;

    class ProjectAssetPool : public AssetPool
    {
    public:
        virtual void Init() override;
        virtual void CleanUp() override;

        void AddAssetToImported(Ref<Asset> asset);
        void AddAssetToRegistry(Ref<Asset> asset, const AssetMetadata& metadata);
        void AddAssetToPool(Ref<Asset> asset, const AssetMetadata& metadata);

        virtual bool IsAssetImported(AssetHandle handle) const override;
        virtual bool IsAssetHandleValid(AssetHandle handle) const override;

        template <typename T>
        Ref<T> GetAsset(AssetHandle handle);

        AssetMetadata& GetAssetMetadata(AssetHandle handle);
        AssetType GetAssetType(AssetHandle handle) const;
        AssetRegistry& GetAssetRegistry() { return m_AssetRegistry; }
        AssetMap& GetAssetsImported() { return m_AssetsImported; }

    private:
        AssetMap m_AssetsImported;
        AssetRegistry m_AssetRegistry;
        std::filesystem::path m_AssetRegistryPath;
    };

}
