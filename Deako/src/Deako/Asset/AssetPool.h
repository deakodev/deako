#pragma once

#include "Deako/Asset/Asset.h"

namespace Deako {

    using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
    using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

    class AssetPoolBase
    {
    public:
        AssetPoolBase(Ref<AssetRegistry> registry)
            : m_AssetRegistry(*registry) {}

        void CleanUp();

        void ImportAssetRegistry();

        Ref<Asset> ImportAsset(AssetHandle handle, AssetMetadata metadata);

        void AddAsset(Ref<Asset> asset, AssetMetadata metadata);

        void RemoveAsset(AssetHandle handle);

        template <typename T>
        Ref<T> GetAsset(AssetHandle handle);

        const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }
        const AssetMap& GetAssetMap() const { return m_AssetsImported; }
        AssetMetadata GetMetadata(AssetHandle handle);

        bool IsAssetHandleValid(AssetHandle handle) const;
        bool IsAssetLoaded(AssetHandle handle) const;
        AssetType GetAssetType(AssetHandle handle) const;

    private:
        AssetRegistry m_AssetRegistry;
        AssetMap m_AssetsImported;
    };

    class AssetPool
    {
    public:
        static Ref<AssetPoolBase> Get();

        static void CleanUp()
        {
            AssetPool::Get()->CleanUp();
        }

        static const AssetRegistry& GetAssetRegistry()
        {
            return AssetPool::Get()->GetAssetRegistry();
        }

        static AssetMetadata GetMetadata(AssetHandle handle)
        {
            return AssetPool::Get()->GetMetadata(handle);
        }

        static void ImportAssetRegistry()
        {
            AssetPool::Get()->ImportAssetRegistry();
        }

        static Ref<Asset> Import(AssetHandle handle, AssetMetadata metadata)
        {
            return AssetPool::Get()->ImportAsset(handle, metadata);
        }

        template <typename T>
        static Ref<T> Import(const std::filesystem::path& path)
        {
            AssetHandle handle;
            AssetMetadata metadata;
            metadata.assetType = AssetTypeFromTypeIndex(std::type_index(typeid(T)));
            metadata.assetPath = path;

            Ref<Asset> asset = AssetPool::Get()->ImportAsset(handle, metadata);
            return std::static_pointer_cast<T>(asset);
        }


        static void AddAsset(Ref<Asset> asset, AssetMetadata metadata)
        {
            return AssetPool::Get()->AddAsset(asset, metadata);
        }

        template <typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            return AssetPool::Get()->GetAsset<T>(handle);
        }

        static void Invalidate(Ref<Asset> asset)
        {
            asset->Invalidate();
        }

        static bool IsAssetHandleValid(AssetHandle handle)
        {
            return AssetPool::Get()->IsAssetHandleValid(handle);
        }

        static bool IsAssetLoaded(AssetHandle handle)
        {
            return AssetPool::Get()->IsAssetLoaded(handle);
        }

        static AssetType GetAssetType(AssetHandle handle)
        {
            return AssetPool::Get()->GetAssetType(handle);
        }

    };
}
