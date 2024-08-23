#include "Asset.h"
#include "dkpch.h"

namespace Deako {

    static const std::map<AssetType, std::string> assetTypeMap = {
            { AssetType::None, "None" },
            { AssetType::Texture2D, "Texture2D" },
            { AssetType::TextureCubeMap, "TextureCubeMap" },
            { AssetType::Material, "Material" },
            { AssetType::Prefab, "Prefab" },
            { AssetType::Model, "Model" },
            { AssetType::Scene, "Scene" },
    };

    const std::string& AssetTypeToString(AssetType type)
    {
        auto it = assetTypeMap.find(type);
        if (it != assetTypeMap.end())
        {
            return it->second;
        }

        static const std::string invalid = "<Invalid>";
        return invalid;
    }

    AssetType AssetTypeFromString(const std::string& type)
    {
        for (const auto& pair : assetTypeMap)
        {
            if (pair.second == type) return pair.first;
        }

        return AssetType::None;
    }

}
