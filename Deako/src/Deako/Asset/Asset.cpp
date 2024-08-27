#include "Asset.h"
#include "dkpch.h"

#include "Deako/Renderer/Vulkan/VulkanTexture.h"
#include "Deako/Renderer/Vulkan/VulkanMaterial.h"
#include "Deako/Renderer/Vulkan/VulkanModel.h"
#include "Deako/Scene/Scene.h"

namespace Deako {

    static const std::map<AssetType, std::string> assetTypeMap = {
            { AssetType::None, "None" },
            { AssetType::Texture2D, "Texture2D" },
            { AssetType::TextureCubeMap, "TextureCubeMap" },
            { AssetType::Material, "Material" },
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

    AssetType AssetTypeFromTypeIndex(const std::type_index& typeIndex)
    {
        static const std::unordered_map<std::type_index, AssetType> typeMap = {
            {std::type_index(typeid(Texture2D)), AssetType::Texture2D},
            {std::type_index(typeid(TextureCubeMap)), AssetType::TextureCubeMap},
            {std::type_index(typeid(Material)), AssetType::Material},
            {std::type_index(typeid(Model)), AssetType::Model},
            {std::type_index(typeid(Scene)), AssetType::Scene},
        };

        auto it = typeMap.find(typeIndex);

        DK_CORE_ASSERT(it != typeMap.end(), "Unsupported asset type!");

        return it->second;
    }


}
