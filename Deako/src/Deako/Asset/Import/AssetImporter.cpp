#include "AssetImporter.h"
#include "dkpch.h"

#include "Deako/Asset/Import/TextureImporter.h"
#include "Deako/Asset/Import/ModelImporter.h"
#include "Deako/Asset/Import/SceneImporter.h"
#include "Deako/Asset/Import/PrefabImporter.h"

namespace Deako {

    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, AssetMetadata)>;
    static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
        { AssetType::Texture2D, TextureImporter::ImportTexture2D },
        { AssetType::TextureCubeMap, TextureImporter::ImportTextureCubeMap },
        { AssetType::Model, ModelImporter::ImportModel },
        { AssetType::Prefab, PrefabImporter::ImportPrefab },
        { AssetType::Scene, SceneImporter::ImportScene }
    };

    Ref<Asset> AssetImporter::Import(AssetHandle handle, AssetMetadata metadata)
    {
        auto it = s_AssetImportFunctions.find(metadata.assetType);
        if (it == s_AssetImportFunctions.end())
        {
            DK_CORE_ERROR("No import function available for asset type: {0}", (uint16_t)metadata.assetType);
            return nullptr;
        }

        return it->second(handle, metadata);
    }

}
