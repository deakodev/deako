#include "RuntimeAssetPool.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"

namespace Deako {

    void RuntimeAssetPool::Init()
    {
        DkContext& deako = Deako::GetContext();
        m_AssetsImported = deako.projectAssetPool->GetAssetsImported();
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
