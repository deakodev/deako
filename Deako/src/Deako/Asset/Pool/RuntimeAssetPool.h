#pragma once

#include "Deako/Asset/Asset.h"
#include "Deako/Asset/Pool/AssetPool.h"

namespace Deako {

    class RuntimeAssetPool : public AssetPool
    {
    public:
        virtual void Init() override;
        virtual void CleanUp() override;

        virtual bool IsAssetHandleValid(AssetHandle handle) const override { return false; }
        virtual bool IsAssetImported(AssetHandle handle) const override { return false; }

    private:
        AssetMap m_AssetsImported;

        inline static Ref<RuntimeAssetPool> s_RuntimeAssetPool = CreateRef<RuntimeAssetPool>();;
    };

}
