#include "ProjectAssetPool.h"
#include "dkpch.h"

#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Asset/Prefab/Prefab.h"

#include "Deako/Project/ProjectHandler.h"
#include "Deako/Project/Serialize.h"
#include "Deako/Project/Deserialize.h"

#include "Deako/Renderer/Vulkan/VulkanTexture.h"
#include "Deako/Renderer/Vulkan/VulkanMaterial.h"
#include "Deako/Renderer/Vulkan/VulkanModel.h"

namespace Deako {

    void ProjectAssetPool::Init()
    {
        DkContext& deako = Deako::GetContext();

        m_AssetRegistryPath = deako.activeProject->workingDirectory /
            deako.activeProject->assetRegistryFilename;

        Deserialize::AssetRegistry(m_AssetRegistry, m_AssetRegistryPath);
    }

    void ProjectAssetPool::CleanUp()
    {
        Serialize::AssetRegistry(m_AssetRegistry, m_AssetRegistryPath);

        if (!m_AssetsImported.empty())
            for (auto it = m_AssetsImported.begin(); it != m_AssetsImported.end(); )
            {
                Ref<Asset> asset = it->second;
                AssetMetadata& metadata = GetAssetMetadata(asset->m_Handle);
                if (asset)
                {
                    DK_CORE_INFO("Destroying: {0}", metadata.assetName);
                    asset->Destroy();
                };

                it = m_AssetsImported.erase(it);
            }
    }

    void ProjectAssetPool::AddAssetToPool(Ref<Asset> asset, const AssetMetadata& metadata)
    {
        m_AssetsImported[asset->m_Handle] = asset;
        m_AssetRegistry[asset->m_Handle] = metadata;
    }

    void ProjectAssetPool::AddAssetToRegistry(Ref<Asset> asset, const AssetMetadata& metadata)
    {
        m_AssetRegistry[asset->m_Handle] = metadata;
    }

    void ProjectAssetPool::AddAssetToImported(Ref<Asset> asset)
    {
        m_AssetsImported[asset->m_Handle] = asset;
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
        DkContext& deako = Deako::GetContext();

        if (!IsAssetHandleValid(handle)) return nullptr;

        Ref<Asset> asset;
        if (IsAssetImported(handle))
        {
            asset = m_AssetsImported.at(handle);
        }
        else
        {
            AssetMetadata metadata = GetAssetMetadata(handle);
            metadata.assetPath = deako.activeProject->assetDirectory / metadata.assetPath;
            asset = AssetManager::ImportAsset(handle, metadata);
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
