#pragma once

#include "Project.h"
#include "Scene.h"
#include "Entity.h"

#include <yaml-cpp/yaml.h>

namespace Deako {

    namespace Serialize {

        bool Project(Deako::Project& project);
        bool Scene(Deako::Scene& scene);
        void Entity(YAML::Emitter& out, Entity entity);

    }

    namespace Deserialize {

        Ref<Project> Project(const std::filesystem::path& path);
        Ref<Scene> Scene(const std::filesystem::path& path);

    }

}
