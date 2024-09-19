#pragma once

#include "Asset.h"
#include "AssetPool.h"

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

    class Prefab : public Asset
    {
    public:
        Prefab() = default;

        virtual ~Prefab() {};

        virtual void Destroy() override {}

        static AssetType GetStaticType() { return AssetType::Prefab; }
        virtual AssetType GetType() const override { return GetStaticType(); }

    public: // think about 
        TextureMap textures;
        MaterialMap materials;
        Ref<Model> model;
    };

}
