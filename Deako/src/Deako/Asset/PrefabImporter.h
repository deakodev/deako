#pragma once

#include "Asset.h"
#include "Prefab.h"

namespace Deako {

    class PrefabImporter
    {
    public:
        static Ref<Prefab> ImportPrefab(AssetHandle handle, AssetMetadata metadata);
        static Ref<Prefab> ImportGLTF(AssetHandle handle, PrefabMetadata metadata);
    };

}
