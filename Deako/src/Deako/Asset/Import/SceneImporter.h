#pragma once

#include "Deako/Asset/Asset.h"

#include "Deako/Scene/Scene.h"

namespace Deako {

    class SceneImporter
    {
    public:
        static Ref<Scene> ImportScene(AssetHandle handle, AssetMetadata metadata);

    };

}
