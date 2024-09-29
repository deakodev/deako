#include "AssetPool.h"
#include "dkpch.h"

#include "Deako/Asset/AssetImporter.h"
#include "Deako/Asset/Prefab.h"

#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"

namespace Deako {

    Ref<AssetPoolBase> AssetPool::Get()
    {
        return Project::GetActive()->GetAssetPool();
    }

    void AssetPoolBase::ImportAssetRegistry()
    {
        for (auto& [handle, metadata] : m_AssetRegistry)
            ImportAsset(handle, metadata);
    }

    void AssetPoolBase::CleanUp()
    {
        for (auto& [handle, asset] : m_AssetsImported)
            RemoveAsset(handle);
    }

    Ref<Asset> AssetPoolBase::ImportAsset(AssetHandle handle, AssetMetadata metadata)
    {
        metadata.assetPath = Project::GetActive()->GetAssetDirectory() / metadata.assetPath;
        Ref<Asset> asset = AssetImporter::Import(handle, metadata);

        if (asset)
        {
            asset->m_Handle = handle;
            AddAsset(asset, metadata);
            return asset;
        }

        return nullptr;
    }

    void AssetPoolBase::AddAsset(Ref<Asset> asset, AssetMetadata metadata)
    {
        m_AssetsImported[asset->m_Handle] = asset;
        m_AssetRegistry[asset->m_Handle] = metadata;
    }

    void AssetPoolBase::RemoveAsset(AssetHandle handle)
    {
        auto loadedIt = m_AssetsImported.find(handle);
        if (loadedIt != m_AssetsImported.end())
        {
            Ref<Asset> asset = loadedIt->second;
            if (asset) asset->Destroy();
        }
    }

    template <typename T>
    Ref<T> AssetPoolBase::GetAsset(AssetHandle handle)
    {
        if (!IsAssetHandleValid(handle)) return nullptr;

        Ref<Asset> asset;
        if (IsAssetLoaded(handle))
        {
            asset = m_AssetsImported.at(handle);
        }
        else
        {
            AssetMetadata metadata = GetMetadata(handle);
            asset = ImportAsset(handle, metadata);
        }

        return std::static_pointer_cast<T>(asset);
    }

    AssetMetadata AssetPoolBase::GetMetadata(AssetHandle handle)
    {
        auto it = m_AssetRegistry.find(handle);
        if (it == m_AssetRegistry.end())
            return AssetMetadata(AssetType::None);

        return it->second;
    }

    bool AssetPoolBase::IsAssetHandleValid(AssetHandle handle) const
    {
        return (handle != 0) && (m_AssetRegistry.find(handle) != m_AssetRegistry.end());
    }

    bool AssetPoolBase::IsAssetLoaded(AssetHandle handle) const
    {
        return m_AssetsImported.find(handle) != m_AssetsImported.end();
    }

    AssetType AssetPoolBase::GetAssetType(AssetHandle handle) const
    {
        if (!IsAssetHandleValid(handle))
            return AssetType::None;

        return m_AssetRegistry.at(handle).assetType;
    }

    template Ref<TextureCubeMap> AssetPoolBase::GetAsset<TextureCubeMap>(AssetHandle handle);
    template Ref<Material> AssetPoolBase::GetAsset<Material>(AssetHandle handle);
    template Ref<Model> AssetPoolBase::GetAsset<Model>(AssetHandle handle);
    template Ref<Scene> AssetPoolBase::GetAsset<Scene>(AssetHandle handle);
    template Ref<Prefab> AssetPoolBase::GetAsset<Prefab>(AssetHandle handle);

}
