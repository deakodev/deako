#pragma once 

#include "Deako/Core/UUID.h"

#include <typeindex>

namespace Deako {

    using AssetHandle = UUID;

    enum class AssetType : uint16_t
    {
        None = 0,
        Texture2D,
        TextureCubeMap,
        Material,
        Model,
        Prefab,
        Scene,
    };

    const std::string& AssetTypeToString(AssetType type);
    AssetType AssetTypeFromString(const std::string& type);
    AssetType AssetTypeFromTypeIndex(const std::type_index& typeIndex);

    struct AssetMetadata
    {
        AssetType assetType{ AssetType::None };
        std::filesystem::path assetPath;

        AssetMetadata() = default;
        AssetMetadata(AssetType type)
            : assetType(type) {}
        virtual ~AssetMetadata() = default;

        operator bool() const { return assetType != AssetType::None; }
    };

    class Asset
    {
    public:
        virtual AssetType GetType() const = 0;

        virtual ~Asset() {};

        virtual void Destroy() = 0;

        AssetHandle m_Handle; // handle generated automatically
    };
}
