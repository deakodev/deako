#pragma once

#include "Deako/Asset/Prefab/Prefab.h"

namespace Deako {

    class PrefabHandler
    {
    public:
        static void Init();
        static void CleanUp();

        static void OpenPrefab();
        static Ref<Prefab> ImportPrefab(const std::filesystem::path& path);
        static Ref<Prefab> ImportPrefab(AssetHandle handle, AssetMetadata& metadata);

        static Ref<Prefab> ParseGLTF(AssetHandle handle, PrefabMetadata metadata);
    };



}
