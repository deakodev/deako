#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Pool/AssetPool.h"

namespace Deako {

    class RuntimeAssetPool : public AssetPool
    {
    public:
        virtual void Init() override;
        virtual void CleanUp() override;

        virtual Ref<Asset> ImportAsset(AssetHandle handle, AssetMetadata metadata) override { return nullptr; }

        virtual bool IsAssetHandleValid(AssetHandle handle) const override { return false; }
        virtual bool IsAssetImported(AssetHandle handle) const override { return false; }

    private:
        AssetMap m_AssetsImported;
    };

}
