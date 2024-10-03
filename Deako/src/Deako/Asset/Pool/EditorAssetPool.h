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

        virtual Ref<Asset> ImportAsset(AssetHandle handle, AssetMetadata metadata) override;

        virtual bool IsAssetImported(AssetHandle handle) const override;
        virtual bool IsAssetHandleValid(AssetHandle handle) const override;

        Ref<Asset> GetEmpty(AssetType assetType);

    private:
        EmptyAssets m_EmptyAssets;
        AssetMap m_AssetsImported;
    };

}
