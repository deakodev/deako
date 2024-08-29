#pragma once 

#include "UUID.h"

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
        Scene,
    };

    const std::string& AssetTypeToString(AssetType type);
    AssetType AssetTypeFromString(const std::string& type);
    AssetType AssetTypeFromTypeIndex(const std::type_index& typeIndex);

    struct AssetMetadata
    {
        std::filesystem::path path;
        AssetType type = AssetType::None;

        operator bool() const { return type != AssetType::None; }
    };

    class Asset
    {
    public:
        virtual AssetType GetType() const = 0;

        AssetHandle m_Handle; // handle generated automatically
    };


}
