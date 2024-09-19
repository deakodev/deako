#pragma once

#include "Asset.h"

namespace Deako {

    class AssetImporter
    {
    public:
        static Ref<Asset> Import(AssetHandle handle, AssetMetadata metadata);
    };

}
