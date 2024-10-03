#pragma once

#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"
#include "Deako/Scene/Entity.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"

#include <yaml-cpp/yaml.h>

namespace Deako {

    namespace Deserialize {

        Ref<Project> Project(const std::filesystem::path& path);

        void AssetRegistry(Deako::AssetRegistry& assetRegistry);

        Ref<Scene> Scene(const std::filesystem::path& path);

    }

}
