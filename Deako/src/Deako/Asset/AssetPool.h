#pragma once

#include "Asset.h"

namespace Deako {

    using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
    using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

    class AssetPool
    {
    public:
        static Ref<Asset> Import(AssetHandle handle, AssetMetadata metadata);

        template <typename T>
        static Ref<T> Import(const std::filesystem::path& path);

        static void Add(Ref<Asset> asset, AssetMetadata metadata);
        static void AddToImported(Ref<Asset> asset);

        template <typename T>
        static Ref<T> Get(AssetHandle handle);

        static void CleanUp();
    };

    class AssetPoolBase
    {
    public:
        AssetPoolBase(Ref<AssetRegistry> registry)
            : m_AssetRegistry(*registry) {}

        Ref<Asset> Import(AssetHandle handle, AssetMetadata metadata);

        void Add(Ref<Asset> asset, AssetMetadata metadata);
        void AddToImported(Ref<Asset> asset);

        void Remove(AssetHandle handle);

        template <typename T>
        Ref<T> Get(AssetHandle handle);

        void CleanUp();

        const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }
        AssetMetadata GetMetadata(AssetHandle handle);

    private:
        bool IsAssetHandleValid(AssetHandle handle) const;
        bool IsAssetLoaded(AssetHandle handle) const;

    private:
        AssetRegistry m_AssetRegistry;
        AssetMap m_AssetsImported;
    };

}
