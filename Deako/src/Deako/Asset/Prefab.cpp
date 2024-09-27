#include "Prefab.h"
#include "dkpch.h"

namespace Deako {

    static const std::map<PrefabType, std::string> prefabTypeMap = {
         { PrefabType::None, "None" },
         { PrefabType::GLTF, "GLTF" },
    };

    const std::string& PrefabTypeToString(PrefabType type)
    {
        auto it = prefabTypeMap.find(type);
        if (it != prefabTypeMap.end())
        {
            return it->second;
        }

        static const std::string invalid = "<Invalid>";
        return invalid;
    }

    PrefabType PrefabTypeFromString(const std::string& type)
    {
        for (const auto& pair : prefabTypeMap)
        {
            if (pair.second == type) return pair.first;
        }

        return PrefabType::None;
    }

}
