#pragma once

#include "Asset.h"

namespace Deako {

    class AssetImporter
    {
    public:
        static Ref<Asset> Import(AssetHandle handle, const AssetMetadata& metadata);
    };

}
