#pragma once

#include "Deako/Asset/Asset.h"

#include "Deako/Renderer/Vulkan/VulkanTexture.h"
#include "Deako/Renderer/Vulkan/VulkanMaterial.h"
#include "Deako/Renderer/Vulkan/VulkanModel.h"

namespace Deako {

    using TextureMap = std::unordered_map<AssetHandle, Ref<Texture2D>>;
    using MaterialMap = std::unordered_map<AssetHandle, Ref<Material>>;

    enum class PrefabType : uint16_t
    {
        None = 0,
        GLTF
    };

    const std::string& PrefabTypeToString(PrefabType type);
    PrefabType PrefabTypeFromString(const std::string& type);

    struct PrefabMetadata : public AssetMetadata
    {
        PrefabType prefabType{ PrefabType::None };

        operator bool() const { return prefabType != PrefabType::None; }
    };

    struct Prefab : public Asset
    {
        TextureMap textures;
        MaterialMap materials;
        Ref<Model> model;

        Prefab() = default;

        virtual void Destroy() override;

        static AssetType GetStaticType() { return AssetType::Prefab; }
        virtual AssetType GetType() const override { return GetStaticType(); }
    };

}
