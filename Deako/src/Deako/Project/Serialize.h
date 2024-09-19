#pragma once

#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"
#include "Deako/Scene/Entity.h"

#include <yaml-cpp/yaml.h>

namespace Deako {

    namespace Serialize {

        bool Project();

        bool AssetRegistry();

        bool Scene(Deako::Scene& scene);

        void Entity(YAML::Emitter& out, Entity entity);

    }

    namespace Deserialize {

        Ref<Project> Project(const std::filesystem::path& path);

        Ref<AssetRegistry> AssetRegistry();

        Ref<Scene> Scene(const std::filesystem::path& path);

    }

}
