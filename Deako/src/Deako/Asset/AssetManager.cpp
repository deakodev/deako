#include "AssetManager.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Pool/EditorAssetPool.h"
#include "Deako/Asset/Pool/RuntimeAssetPool.h"

#include "Deako/Asset/Texture/TextureHandler.h"
#include "Deako/Asset/Mesh/MeshHandler.h"
#include "Deako/Asset/Prefab/PrefabHandler.h"
#include "Deako/Asset/Scene/SceneHandler.h"

namespace Deako {

    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, AssetMetadata&)>;
    std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
     { AssetType::Texture2D, [](AssetHandle handle, AssetMetadata& metadata) {
         return AssetManager::CastToAsset(TextureHandler::ImportTexture2D(handle, metadata));
     }},
     { AssetType::TextureCubeMap, [](AssetHandle handle, AssetMetadata& metadata) {
         return AssetManager::CastToAsset(TextureHandler::ImportTextureCubeMap(handle, metadata));
     }},
     { AssetType::Model, [](AssetHandle handle, AssetMetadata& metadata) {
         return AssetManager::CastToAsset(MeshHandler::ImportMesh(handle, metadata));
     }},
     { AssetType::Prefab, [](AssetHandle handle, AssetMetadata& metadata) {
         return AssetManager::CastToAsset(PrefabHandler::ImportPrefab(handle, metadata));
     }},
     { AssetType::Scene, [](AssetHandle handle, AssetMetadata& metadata) {
         return AssetManager::CastToAsset(SceneHandler::ImportScene(handle, metadata));
     }}
    };

    void AssetManager::Init()
    {
        ProjectAssetPool::Get()->Init();
        EditorAssetPool::Get()->Init();
        // RuntimeAssetPool::Get()->Init();

        TextureHandler::Init();
        MeshHandler::Init();
        PrefabHandler::Init();
        SceneHandler::Init();
    }

    void AssetManager::CleanUp()
    {
        ProjectAssetPool::Get()->CleanUp();
        EditorAssetPool::Get()->CleanUp();
        // RuntimeAssetPool::Get()->CleanUp();

        TextureHandler::CleanUp();
        MeshHandler::CleanUp();
        PrefabHandler::CleanUp();
        SceneHandler::CleanUp();
    }

    Ref<Asset> AssetManager::ImportAsset(AssetHandle handle, AssetMetadata& metadata)
    {
        auto it = s_AssetImportFunctions.find(metadata.assetType);
        if (it == s_AssetImportFunctions.end())
        {
            DK_CORE_ERROR("No import function available for asset type: {0}", (uint16_t)metadata.assetType);
            return nullptr;
        }

        Ref<Asset> asset = it->second(handle, metadata);

        return asset;
    }

    template<typename T>
    Ref<Asset> AssetManager::CastToAsset(Ref<T> asset)
    {
        return std::dynamic_pointer_cast<Asset>(asset);  // Dynamic cast for safety
    }


}
