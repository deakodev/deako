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
            asset = AssetImporter::Import(handle, metadata);
        }

        return asset;
    }

    Ref<Asset> EditorAssetPool::ImportAsset(const AssetType& type, const std::filesystem::path& path)
    {
        AssetHandle handle; // generates handle
        AssetMetadata metadata;
        metadata.path = path;
        metadata.type = type;

        Ref<Asset> asset = AssetImporter::Import(handle, metadata);
        asset->m_Handle = handle;

        if (asset)
        {
            m_LoadedAssets[handle] = asset;
            m_AssetRegistry[handle] = metadata;
            Serialize::AssetRegistry();
        }

        return asset;
    }

}
