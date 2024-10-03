#pragma once

#include "Deako/Asset/Asset.h"

#include "Deako/Asset/Pool/EditorAssetPool.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Pool/RuntimeAssetPool.h"

namespace Deako {

    class AssetManager
    {
    public:
        static void Init();
        static void CleanUp();

        static Ref<EditorAssetPool> GetEditorAssetPool() { return s_EditorAssetPool; }
        static Ref<ProjectAssetPool> GetProjectAssetPool() { return s_ProjectAssetPool; }
        static Ref<RuntimeAssetPool> GetRuntimeAssetPool() { return s_RuntimeAssetPool; }

    private:
        inline static Ref<EditorAssetPool> s_EditorAssetPool;
        inline static Ref<ProjectAssetPool> s_ProjectAssetPool;
        inline static Ref<RuntimeAssetPool> s_RuntimeAssetPool;
    };
}
