#include "AssetPool.h"
#include "dkpch.h"

#include "AssetImporter.h"
#include "Prefab.h"

#include "Deako/Project/Serialize.h"
#include "Deako/Renderer/Vulkan/VulkanModel.h"

#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"

namespace Deako {

    template <>
    Ref<Model> AssetPool::Import<Model>(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Model;
        metadata.assetPath = Project::GetAssetDirectory() / path;

        Ref<Asset> asset = Import(handle, metadata);
        return std::static_pointer_cast<Model>(asset);
    }

    template <>
    Ref<Scene> AssetPool::Import<Scene>(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Scene;
        metadata.assetPath = path;

        Ref<Asset> asset = Import(handle, metadata);
        return std::static_pointer_cast<Scene>(asset);
    }

    template <>
    Ref<Texture2D> AssetPool::Import<Texture2D>(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::Texture2D;
        metadata.assetPath = Project::GetAssetDirectory() / path;

        Ref<Asset> asset = Import(handle, metadata);
        return std::static_pointer_cast<Texture2D>(asset);
    }

    template <>
    Ref<TextureCubeMap> AssetPool::Import<TextureCubeMap>(const std::filesystem::path& path)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.assetType = AssetType::TextureCubeMap;
        metadata.assetPath = Project::GetAssetDirectory() / path;

        Ref<Asset> asset = Import(handle, metadata);
        return std::static_pointer_cast<TextureCubeMap>(asset);
    }

    Ref<Asset> AssetPool::Import(AssetHandle handle, AssetMetadata metadata)
    {
        Ref<Asset> asset = Project::GetActive()->GetAssetPool()->Import(handle, metadata);
        AddToImported(asset);
        return asset;
    }

    void AssetPool::Add(Ref<Asset> asset, AssetMetadata metadata)
    {
        Project::GetActive()->GetAssetPool()->Add(asset, metadata);
    }

    void AssetPool::AddToImported(Ref<Asset> asset)
    {
        Project::GetActive()->GetAssetPool()->AddToImported(asset);
    }

    template <>
    Ref<Texture2D> AssetPool::Get<Texture2D>(AssetHandle handle)
    {
        return Project::GetActive()->GetAssetPool()->Get<Texture2D>(handle);
    }

    template <>
    Ref<TextureCubeMap> AssetPool::Get<TextureCubeMap>(AssetHandle handle)
    {
        return Project::GetActive()->GetAssetPool()->Get<TextureCubeMap>(handle);
    }

    template <>
    Ref<Model> AssetPool::Get<Model>(AssetHandle handle)
    {
        return Project::GetActive()->GetAssetPool()->Get<Model>(handle);
    }

    template <>
    Ref<Prefab> AssetPool::Get<Prefab>(AssetHandle handle)
    {
        return Project::GetActive()->GetAssetPool()->Get<Prefab>(handle);
    }

    template <>
    Ref<Scene> AssetPool::Get<Scene>(AssetHandle handle)
    {
        return Project::GetActive()->GetAssetPool()->Get<Scene>(handle);
    }

    void AssetPool::CleanUp()
    {
        Project::GetActive()->GetAssetPool()->CleanUp();
    }


    Ref<Asset> AssetPoolBase::Import(AssetHandle handle, AssetMetadata metadata)
    {
        return AssetImporter::Import(handle, metadata);
    }

    void AssetPoolBase::Add(Ref<Asset> asset, AssetMetadata metadata)
    {
        m_AssetsImported[asset->m_Handle] = asset;
        m_AssetRegistry[asset->m_Handle] = metadata;
    }

    void AssetPoolBase::AddToImported(Ref<Asset> asset)
    {
        m_AssetsImported[asset->m_Handle] = asset;
    }

    void AssetPoolBase::Remove(AssetHandle handle)
    {
        auto loadedIt = m_AssetsImported.find(handle);
        if (loadedIt != m_AssetsImported.end())
        {
            Ref<Asset> asset = loadedIt->second;
            if (asset) asset->Destroy();
        }
    }

    template <typename T>
    Ref<T> AssetPoolBase::Get(AssetHandle handle)
    {
        if (!IsAssetHandleValid(handle)) return nullptr;

        Ref<Asset> asset;
        if (IsAssetLoaded(handle))
        {
            asset = m_AssetsImported.at(handle);
        }
        else
        {
            AssetMetadata metadata = GetMetadata(handle);
            metadata.assetPath = Project::GetAssetDirectory() / metadata.assetPath;
            asset = Import(handle, metadata);
        }

        return std::static_pointer_cast<T>(asset);
    }

    void AssetPoolBase::CleanUp()
    {
        for (auto& [handle, asset] : m_AssetsImported)
            Remove(handle);
    }

    AssetMetadata AssetPoolBase::GetMetadata(AssetHandle handle)
    {
        auto it = m_AssetRegistry.find(handle);
        if (it == m_AssetRegistry.end())
            return AssetMetadata(AssetType::None);

        return it->second;
    }

    bool AssetPoolBase::IsAssetHandleValid(AssetHandle handle) const
    {
        return (handle != 0) && (m_AssetRegistry.find(handle) != m_AssetRegistry.end());
    }

    bool AssetPoolBase::IsAssetLoaded(AssetHandle handle) const
    {
        return m_AssetsImported.find(handle) != m_AssetsImported.end();
    }

}
