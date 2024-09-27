#pragma once

#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"
#include "Deako/Scene/Entity.h"

#include <yaml-cpp/yaml.h>

namespace Deako {

    namespace Deserialize {

        Ref<Project> Project(const std::filesystem::path& path);

        Ref<AssetRegistry> AssetRegistry(const std::filesystem::path& path);

        Ref<Scene> Scene(const std::filesystem::path& path);

    }

}
