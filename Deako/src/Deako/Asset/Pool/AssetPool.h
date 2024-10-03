#pragma once

#include "Deako/Asset/Asset.h"

namespace Deako {

    using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

    class AssetPool
    {
    public:
        virtual void Init() = 0;
        virtual void CleanUp() = 0;

        virtual ~AssetPool() = default;

        virtual Ref<Asset> ImportAsset(AssetHandle handle, AssetMetadata metadata) = 0;

        virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
        virtual bool IsAssetImported(AssetHandle handle) const = 0;
    };

}
