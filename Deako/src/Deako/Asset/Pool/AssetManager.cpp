#include "AssetManager.h"
#include "dkpch.h"

namespace Deako {

    void AssetManager::Init()
    {
        s_ProjectAssetPool = CreateRef<ProjectAssetPool>();
        s_ProjectAssetPool->Init();

        s_EditorAssetPool = CreateRef<EditorAssetPool>();
        s_EditorAssetPool->Init();

        s_RuntimeAssetPool = CreateRef<RuntimeAssetPool>();
        // s_RuntimeAssetPool->Init();
    }

    void AssetManager::CleanUp()
    {
        s_EditorAssetPool->CleanUp();
        s_ProjectAssetPool->CleanUp();
        // s_RuntimeAssetPool->CleanUp();
    }
}
