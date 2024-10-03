#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Pool/AssetPool.h"

namespace Deako {

    using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;

    class ProjectAssetPool : public AssetPool
    {
    public:
        virtual void Init() override;
        virtual void CleanUp() override;

        virtual Ref<Asset> ImportAsset(AssetHandle handle, AssetMetadata metadata) override;
        void ImportAsset(const std::filesystem::path path);

        virtual bool IsAssetImported(AssetHandle handle) const override;
        virtual bool IsAssetHandleValid(AssetHandle handle) const override;

        template <typename T>
        Ref<T> GetAsset(AssetHandle handle);

        AssetMetadata& GetAssetMetadata(AssetHandle handle);
        AssetType GetAssetType(AssetHandle handle) const;
        AssetRegistry& GetAssetRegistry() { return m_AssetRegistry; }
        AssetMap& GetAssetsImported() { return m_AssetsImported; }

    private:
        AssetRegistry m_AssetRegistry;
        AssetMap m_AssetsImported;
    };

}
