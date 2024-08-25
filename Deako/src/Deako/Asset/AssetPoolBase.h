#pragma once

#include "Asset.h"

namespace Deako {

    using AssetRegistry = std::map<AssetHandle, AssetMetadata>;
    using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

    class AssetPoolBase
    {
    public:
        virtual Ref<Asset> GetAsset(AssetHandle handle) const = 0;
        virtual void DestroyAsset(AssetHandle handle) = 0;

        virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
        virtual bool IsAssetLoaded(AssetHandle handle) const = 0;

    };

    class EditorAssetPool : public AssetPoolBase
    {
    public:
        EditorAssetPool(Ref<AssetRegistry> registry)
            : m_AssetRegistry(*registry) {}

        virtual Ref<Asset> GetAsset(AssetHandle handle) const override;
        virtual void DestroyAsset(AssetHandle handle) override;

        virtual bool IsAssetHandleValid(AssetHandle handle) const override;
        virtual bool IsAssetLoaded(AssetHandle handle) const override;

        Ref<Asset> ImportAsset(const AssetType& type, const std::filesystem::path& path);

        const AssetMetadata& GetMetadata(AssetHandle handle) const;

        const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

    private:
        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;
        // AssetMap m_MemoryAssets; TODO
    };

    class RuntimeAssetPool : public AssetPoolBase
    {
    public:

    private:

    };

}
