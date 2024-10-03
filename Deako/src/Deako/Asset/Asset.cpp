#include "Asset.h"
#include "dkpch.h"

#include "Deako/Renderer/Vulkan/VulkanTexture.h"
#include "Deako/Renderer/Vulkan/VulkanMaterial.h"
#include "Deako/Renderer/Vulkan/VulkanModel.h"
#include "Deako/Asset/Prefab.h"
#include "Deako/Scene/Scene.h"

namespace Deako {

    static const std::map<AssetType, std::string> assetTypeStringMap = {
            { AssetType::None, "None" },
            { AssetType::Texture2D, "Texture2D" },
            { AssetType::TextureCubeMap, "TextureCubeMap" },
            { AssetType::Material, "Material" },
            { AssetType::Model, "Model" },
            { AssetType::Prefab, "Prefab" },
            { AssetType::Scene, "Scene" },
    };

    static const std::map<AssetType, std::string> assetTypeDirectoryMap = {
            { AssetType::None, "" },
            { AssetType::Texture2D, "textures" },
            { AssetType::TextureCubeMap, "environments" },
            { AssetType::Material, "materials" },
            { AssetType::Model, "models" },
            { AssetType::Prefab, "prefabs" },
            { AssetType::Scene, "scenes" },
    };

    const std::string& AssetTypeToString(AssetType type)
    {
        auto it = assetTypeStringMap.find(type);
        if (it != assetTypeStringMap.end())
        {
            return it->second;
        }

        static const std::string invalid = "<Invalid>";
        return invalid;
    }

    AssetType AssetTypeFromString(const std::string& type)
    {
        for (const auto& pair : assetTypeStringMap)
        {
            if (pair.second == type) return pair.first;
        }

        return AssetType::None;
    }

    AssetType AssetTypeFromParentDirectory(const std::string& fileParentDirectory)
    {
        for (const auto& pair : assetTypeDirectoryMap)
        {
            if (pair.second == fileParentDirectory) return pair.first;
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
            {std::type_index(typeid(Prefab)), AssetType::Prefab},
            {std::type_index(typeid(Scene)), AssetType::Scene},
        };

        auto it = typeMap.find(typeIndex);

        DK_CORE_ASSERT(it != typeMap.end(), "Unsupported asset type!");

        return it->second;
    }

}
