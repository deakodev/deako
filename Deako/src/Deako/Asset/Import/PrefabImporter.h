#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Prefab.h"

namespace Deako {

    class PrefabImporter
    {
    public:
        static Ref<Prefab> ImportPrefab(AssetHandle handle, AssetMetadata metadata);
        static Ref<Prefab> ImportGLTF(AssetHandle handle, PrefabMetadata metadata);
    };

}
