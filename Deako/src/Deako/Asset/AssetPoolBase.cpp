#include "AssetPoolBase.h"
#include "dkpch.h"

#include "AssetImporter.h"

#include "Deako/Project/Serialize.h"

namespace Deako {

    bool EditorAssetPool::IsAssetHandleValid(AssetHandle handle) const
    {
        return (handle != 0) && (m_AssetRegistry.find(handle) != m_AssetRegistry.end());
    }

    bool EditorAssetPool::IsAssetLoaded(AssetHandle handle) const
    {
        return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
    }

    const AssetMetadata& EditorAssetPool::GetMetadata(AssetHandle handle) const
    {
        static AssetMetadata s_NullMetadata;
        auto it = m_AssetRegistry.find(handle);
        if (it == m_AssetRegistry.end())
            return s_NullMetadata;

        return it->second;
    }

    Ref<Asset> EditorAssetPool::GetAsset(AssetHandle handle) const
    {
        if (!IsAssetHandleValid(handle)) return nullptr;

        Ref<Asset> asset;
        if (!IsAssetLoaded(handle))
        {
            asset = m_LoadedAssets.at(handle);
        }
        else
        {
            const AssetMetadata& metadata = GetMetadata(handle);
            asset = AssetImporter::Import(metadata);
        }

        return asset;
    }

    void EditorAssetPool::DestroyAsset(AssetHandle handle)
    {
        const AssetMetadata& metadata = GetMetadata(handle);

        auto loadedIt = m_LoadedAssets.find(handle);
        if (loadedIt != m_LoadedAssets.end())
        {
            Ref<Asset> asset = loadedIt->second;

            if (metadata.type == AssetType::Model)
            {
                auto model = std::dynamic_pointer_cast<Model>(asset);
                model->Destroy();
            }
            m_LoadedAssets.erase(loadedIt);
        }

        auto registryIt = m_AssetRegistry.find(handle);
        if (registryIt != m_AssetRegistry.end())
        {
            m_AssetRegistry.erase(registryIt);
        }
    }

    Ref<Asset> EditorAssetPool::ImportAsset(const AssetType& type, const std::filesystem::path& path)
    {
        AssetHandle handle; // generates handle
        AssetMetadata metadata;
        metadata.path = path;
        metadata.type = type;

        Ref<Asset> asset = AssetImporter::Import(metadata);
        asset->m_Handle = handle;

        if (asset) AddAsset(asset, metadata);

        return asset;
    }

    void EditorAssetPool::AddAsset(Ref<Asset> asset, const AssetMetadata& metadata)
    {
        m_LoadedAssets[asset->m_Handle] = asset;
        m_AssetRegistry[asset->m_Handle] = metadata;
        Serialize::AssetRegistry();
    }

}
