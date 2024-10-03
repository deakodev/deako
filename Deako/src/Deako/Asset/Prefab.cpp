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

    void Prefab::Destroy()
    {
        // model->Destroy();

        // if (model)
        // {
        //     model->Destroy();
        //     model.reset();  // Optionally reset the model reference to free it
        // }

        // // Destroy each texture in the textures map
        // for (auto& [handle, texture] : textures)
        // {
        //     if (texture)
        //     {
        //         texture->Destroy();
        //         texture.reset();  // Reset the reference to release the texture
        //     }
        // }
        // textures.clear();  // Clear the map to release the handles

        // // Destroy each material in the materials map
        // for (auto& [handle, material] : materials)
        // {
        //     if (material)
        //     {
        //         material->Destroy();
        //         material.reset();  // Reset the reference to release the material
        //     }
        // }
        // materials.clear();
    }

}
