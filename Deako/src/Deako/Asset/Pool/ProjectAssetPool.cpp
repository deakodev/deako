#include "ProjectAssetPool.h"
#include "dkpch.h"

#include "Deako/Project/Project.h"
#include "Deako/Project/Serialize.h"
#include "Deako/Project/Deserialize.h"
#include "Deako/Asset/Import/AssetImporter.h"

#include "Deako/Scene/Scene.h"
#include "Deako/Asset/Prefab.h"
#include "Deako/Renderer/Vulkan/VulkanTexture.h"
#include "Deako/Renderer/Vulkan/VulkanMaterial.h"
#include "Deako/Renderer/Vulkan/VulkanModel.h"

namespace Deako {

    void ProjectAssetPool::Init()
    {
        Deserialize::AssetRegistry(m_AssetRegistry);
    }

    void ProjectAssetPool::CleanUp()
    {
        if (!m_AssetsImported.empty())
            for (auto it = m_AssetsImported.begin(); it != m_AssetsImported.end(); )
            {
                Ref<Asset> asset = it->second;
                if (asset) asset->Destroy();

                it = m_AssetsImported.erase(it);
            }
    }

    Ref<Asset> ProjectAssetPool::ImportAsset(AssetHandle handle, AssetMetadata metadata)
    {
        metadata.assetPath = Project::GetActive()->GetAssetDirectory() / metadata.assetPath;
        Ref<Asset> asset = AssetImporter::Import(handle, metadata);

        if (asset)
        {
            asset->m_Handle = handle;
            m_AssetsImported[asset->m_Handle] = asset;
            m_AssetRegistry[asset->m_Handle] = metadata;

            if (metadata.assetType == AssetType::Prefab)
            {
                Ref<Prefab> prefab = std::static_pointer_cast<Prefab>(asset);

                for (auto& [handle, texture] : prefab->textures)
                {
                    m_AssetsImported[handle] = texture;
                    metadata.assetType = AssetType::Texture2D;
                    metadata.assetPath = "";
                    m_AssetRegistry[handle] = metadata;
                }

                for (auto& [handle, material] : prefab->materials)
                {
                    m_AssetsImported[handle] = material;
                    metadata.assetType = AssetType::Material;
                    metadata.assetPath = "";
                    m_AssetRegistry[handle] = metadata;
                }

                {
                    m_AssetsImported[prefab->model->m_Handle] = prefab->model;
                    metadata.assetType = AssetType::Model;
                    metadata.assetPath = "";
                    m_AssetRegistry[prefab->model->m_Handle] = metadata;
                }
            }

            Serialize::AssetRegistry(m_AssetRegistry);
            return asset;
        }

        return nullptr;
    }

    void ProjectAssetPool::ImportAsset(const std::filesystem::path path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetPath = Project::GetActive()->GetAssetDirectory() / path;
        metadata.assetType = AssetTypeFromParentDirectory(path.parent_path().filename().string());

        Ref<Asset> asset = AssetImporter::Import(handle, metadata);

        if (asset)
        {
            asset->m_Handle = handle;
            m_AssetsImported[handle] = asset;
            m_AssetRegistry[handle] = metadata;
            Serialize::AssetRegistry(m_AssetRegistry);
        }
    }

    bool ProjectAssetPool::IsAssetImported(AssetHandle handle) const
    {
        return m_AssetsImported.find(handle) != m_AssetsImported.end();
    }

    bool ProjectAssetPool::IsAssetHandleValid(AssetHandle handle) const
    {
        return (handle != 0) && (m_AssetRegistry.find(handle) != m_AssetRegistry.end());
    }

    template <typename T>
    Ref<T> ProjectAssetPool::GetAsset(AssetHandle handle)
    {
        if (!IsAssetHandleValid(handle)) return nullptr;

        Ref<Asset> asset;
        if (IsAssetImported(handle))
        {
            asset = m_AssetsImported.at(handle);
        }
        else
        {
            AssetMetadata metadata = GetAssetMetadata(handle);
            asset = ImportAsset(handle, metadata);
        }

        return std::static_pointer_cast<T>(asset);
    }

    AssetMetadata& ProjectAssetPool::GetAssetMetadata(AssetHandle handle)
    {
        auto it = m_AssetRegistry.find(handle);
        if (it == m_AssetRegistry.end())
        {
            static AssetMetadata invalidMetadata(AssetType::None);
            return invalidMetadata;
        }

        return it->second;
    }

    AssetType ProjectAssetPool::GetAssetType(AssetHandle handle) const
    {
        if (!IsAssetHandleValid(handle))
            return AssetType::None;

        return m_AssetRegistry.at(handle).assetType;
    }

    template Ref<TextureCubeMap> ProjectAssetPool::GetAsset<TextureCubeMap>(AssetHandle handle);
    template Ref<Material> ProjectAssetPool::GetAsset<Material>(AssetHandle handle);
    template Ref<Model> ProjectAssetPool::GetAsset<Model>(AssetHandle handle);
    template Ref<Scene> ProjectAssetPool::GetAsset<Scene>(AssetHandle handle);
    template Ref<Prefab> ProjectAssetPool::GetAsset<Prefab>(AssetHandle handle);

}
