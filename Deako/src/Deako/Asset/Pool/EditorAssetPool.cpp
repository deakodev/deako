#include "EditorAssetPool.h"
#include "dkpch.h"

#include "Deako/Asset/Import/AssetImporter.h"
#include "Deako/Asset/Prefab.h"

namespace Deako {

    void EditorAssetPool::Init()
    {
        {   // empty texture 2d
            AssetHandle handle;
            AssetMetadata metadata;
            metadata.assetType = AssetType::Texture2D;
            metadata.assetPath = "empty/emptyTexture.ktx";
            Ref<Asset> emptyTexture2D = ImportAsset(handle, metadata);
            m_EmptyAssets[AssetType::Texture2D] = emptyTexture2D;
        }

        {   // empty texture cube map
            AssetHandle handle;
            AssetMetadata metadata;
            metadata.assetType = AssetType::TextureCubeMap;
            metadata.assetPath = "empty/emptyCubeMap.ktx";
            Ref<Asset> emptyTextureCubeMap = ImportAsset(handle, metadata);
            m_EmptyAssets[AssetType::TextureCubeMap] = emptyTextureCubeMap;
        }

        {   // empty scene
            AssetHandle handle;
            AssetMetadata metadata;
            metadata.assetType = AssetType::Scene;
            metadata.assetPath = "empty/emptyScene.dscene";
            Ref<Asset> emptyScene = ImportAsset(handle, metadata);
            m_EmptyAssets[AssetType::Scene] = emptyScene;
        }
    }

    void EditorAssetPool::CleanUp()
    {
        if (!m_AssetsImported.empty())
            for (auto it = m_AssetsImported.begin(); it != m_AssetsImported.end(); )
            {
                Ref<Asset> asset = it->second;
                if (asset) asset->Destroy();

                it = m_AssetsImported.erase(it);
            }
    }

    Ref<Asset> EditorAssetPool::ImportAsset(AssetHandle handle, AssetMetadata metadata)
    {
        metadata.assetPath = "Deako/assets" / metadata.assetPath;
        Ref<Asset> asset = AssetImporter::Import(handle, metadata);

        if (asset)
        {
            asset->m_Handle = handle;
            m_AssetsImported[asset->m_Handle] = asset;

            if (metadata.assetType == AssetType::Prefab)
            {
                Ref<Prefab> prefab = std::static_pointer_cast<Prefab>(asset);
                m_AssetsImported[prefab->model->m_Handle] = prefab->model;

                for (auto& [handle, texture] : prefab->textures)
                    m_AssetsImported[handle] = texture;

                for (auto& [handle, material] : prefab->materials)
                    m_AssetsImported[handle] = material;
            }

            return asset;
        }

        return nullptr;
    }

    bool EditorAssetPool::IsAssetImported(AssetHandle handle) const
    {
        return m_AssetsImported.find(handle) != m_AssetsImported.end();
    }

    bool EditorAssetPool::IsAssetHandleValid(AssetHandle handle) const
    {
        return false;
    }

    Ref<Asset> EditorAssetPool::GetEmpty(AssetType assetType)
    {
        auto it = m_EmptyAssets.find(assetType);
        DK_CORE_ASSERT(it != m_EmptyAssets.end(), "Failed to find empty asset!");
        return it->second;
    }

}
