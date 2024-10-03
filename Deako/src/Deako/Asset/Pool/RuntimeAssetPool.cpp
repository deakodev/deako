#include "RuntimeAssetPool.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/AssetManager.h"

namespace Deako {

    void RuntimeAssetPool::Init()
    {
        m_AssetsImported = AssetManager::GetProjectAssetPool()->GetAssetsImported();
    }

    void RuntimeAssetPool::CleanUp()
    {
        if (!m_AssetsImported.empty())
            for (auto it = m_AssetsImported.begin(); it != m_AssetsImported.end(); )
            {
                Ref<Asset> asset = it->second;
                if (asset) asset->Destroy();

                it = m_AssetsImported.erase(it);
            }
    }

}
