#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Pool/AssetPool.h"

namespace Deako {

    using EmptyAssets = std::map<AssetType, Ref<Asset>>;

    class EditorAssetPool : public AssetPool
    {
    public:
        virtual void Init() override;
        virtual void CleanUp() override;

        void AddAssetToPool(Ref<Asset> asset);

        virtual bool IsAssetImported(AssetHandle handle) const override;
        virtual bool IsAssetHandleValid(AssetHandle handle) const override;

    private:
        AssetMap m_AssetsImported;
    };

}
