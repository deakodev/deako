#pragma once 

#include "Deako/Core/Handle.h"

#include <typeindex>

namespace Deako {

    using AssetHandle = DkHandle;

    enum class AssetType : DkU16
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
        std::string assetName;
        AssetType assetType{ AssetType::None };
        std::filesystem::path assetPath;
        AssetHandle parentAssetHandle = 0;

        AssetMetadata() = default;
        AssetMetadata(AssetType type)
            : assetType(type) {
        }
        virtual ~AssetMetadata() = default;

        operator bool() const { return assetType != AssetType::None; }
    };

    class Asset
    {
    public:
        virtual AssetType GetType() const = 0;

        virtual void Invalidate() {};

        virtual void Destroy() = 0;

        AssetHandle m_Handle; // handle generated automatically
    };
}
