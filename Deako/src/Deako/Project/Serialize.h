#pragma once

#include "Deako/Project/Project.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Asset/Scene/Entity.h"

#include <yaml-cpp/yaml.h>

namespace Deako {

    namespace Serialize {

        bool Project(Deako::Project& project, const std::filesystem::path& path);

        bool AssetRegistry(Deako::AssetRegistry& assetRegistry, const std::filesystem::path& path);

        bool Scene(Deako::Scene& scene, AssetMetadata& metadata);

        void Entity(YAML::Emitter& out, EntityHandle handle);

    }

}

namespace YAML {

    template<>
    struct convert<Deako::DkVec3>
    {
        static Node encode(const Deako::DkVec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, Deako::DkVec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<Deako::DkF32>();
            rhs.y = node[1].as<Deako::DkF32>();
            rhs.z = node[2].as<Deako::DkF32>();
            return true;
        }
    };

    template<>
    struct convert<Deako::DkVec4>
    {
        static Node encode(const Deako::DkVec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, Deako::DkVec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<Deako::DkF32>();
            rhs.y = node[1].as<Deako::DkF32>();
            rhs.z = node[2].as<Deako::DkF32>();
            rhs.w = node[3].as<Deako::DkF32>();
            return true;
        }
    };

}

