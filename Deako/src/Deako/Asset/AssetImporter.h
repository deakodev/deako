#pragma once

#include "Asset.h"

namespace Deako {

    class AssetImporter
    {
    public:
        static Ref<Asset> Import(const AssetMetadata& metadata);
    };

}
