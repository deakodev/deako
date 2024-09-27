#pragma once

#include "Deako/Asset/Asset.h"

namespace Deako {

    class AssetImporter
    {
    public:
        static Ref<Asset> Import(AssetHandle handle, AssetMetadata metadata);
    };

}
