#include "EditorAssetPool.h"
#include "dkpch.h"

#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Prefab/Prefab.h"
#include "Deako/Asset/Scene/Scene.h"

namespace Deako {

    void EditorAssetPool::Init()
    {
    }

    void EditorAssetPool::CleanUp()
    {
        if (!m_AssetsImported.empty())
            for (auto it = m_AssetsImported.begin(); it != m_AssetsImported.end(); )
            {
                Ref<Asset> asset = it->second;
                AssetType assetType = asset->GetType();

                DK_CORE_INFO("Destroying: {0}", AssetTypeToString(assetType));
                if (asset) asset->Destroy();

                it = m_AssetsImported.erase(it);
            }
    }

    void EditorAssetPool::AddAssetToPool(Ref<Asset> asset)
    {
        m_AssetsImported[asset->m_Handle] = asset;
    }

    bool EditorAssetPool::IsAssetImported(AssetHandle handle) const
    {
        return m_AssetsImported.find(handle) != m_AssetsImported.end();
    }

    bool EditorAssetPool::IsAssetHandleValid(AssetHandle handle) const
    {
        return false;
    }

}
