#pragma once

#include "Deako/Project/Project.h"
#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Asset/Scene/Entity.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"

#include <yaml-cpp/yaml.h>

namespace Deako {

    namespace Deserialize {

        bool Project(Deako::Project& project, const std::filesystem::path& path);

        void AssetRegistry(Deako::AssetRegistry& assetRegistry, const std::filesystem::path& path);

        void Scene(Deako::Scene& scene, AssetMetadata& metadata);

    }

}
