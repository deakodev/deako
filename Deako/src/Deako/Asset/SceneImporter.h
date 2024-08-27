#pragma once

#include "Asset.h"

#include "Deako/Scene/Scene.h"

namespace Deako {

    class SceneImporter
    {
    public:
        static Ref<Scene> ImportScene(const AssetMetadata& metadata);

    };

}
