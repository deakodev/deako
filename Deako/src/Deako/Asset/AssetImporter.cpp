#include "AssetImporter.h"
#include "dkpch.h"

#include "TextureImporter.h"
#include "SceneImporter.h"

namespace Deako {

    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
    static std::map <AssetType, AssetImportFunction> s_AssetImportFunctions = {
        { AssetType::Texture2D, TextureImporter::ImportTexture2D },
        { AssetType::TextureCubeMap, TextureImporter::ImportTextureCubeMap },
        { AssetType::Scene, SceneImporter::ImportScene }
    };

    Ref<Asset> AssetImporter::Import(AssetHandle handle, const AssetMetadata& metadata)
    {
        auto it = s_AssetImportFunctions.find(metadata.type);
        if (it == s_AssetImportFunctions.end())
        {
            DK_CORE_ERROR("No import function available for asset type: {0}", (uint16_t)metadata.type);
            return nullptr;
        }

        return it->second(handle, metadata);
    }

}
