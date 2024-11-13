#pragma once

#include "Deako/Asset/Asset.h"

namespace Deako {

    class AssetManager
    {
    public:
        static void Init();
        static void CleanUp();

        static Ref<Asset> ImportAsset(AssetHandle handle, AssetMetadata& metadata);

        template<typename T>
        static Ref<Asset> CastToAsset(Ref<T> asset);

        template<typename T>
        static Scope<Asset> CastToAsset(Scope<T> asset);
    };
}
