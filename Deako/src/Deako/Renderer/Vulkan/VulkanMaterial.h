#pragma once


#include "VulkanTexture.h"
#include "Deako/Asset/Asset.h"

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    enum PBRWorkflows { PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSSINESS = 1 };

    struct alignas(16) ShaderMaterial
    {
        DkVec4 baseColorFactor;
        DkVec4 emissiveFactor;
        DkVec4 diffuseFactor;
        DkVec4 specularFactor;
        DkF32 workflow;
        int colorTextureSet;
        int PhysicalDescriptorTextureSet;
        int normalTextureSet;
        int occlusionTextureSet;
        int emissiveTextureSet;
        DkF32 metallicFactor;
        DkF32 roughnessFactor;
        DkF32 alphaMask;
        DkF32 alphaMaskCutoff;
        DkF32 emissiveStrength;
    };

    struct Material : public Asset
    {
        enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
        AlphaMode alphaMode{ ALPHAMODE_OPAQUE };

        DkF32 alphaCutoff{ 1.0f };
        DkF32 metallicFactor{ 1.0f };
        DkF32 roughnessFactor{ 1.0f };
        DkVec4 baseColorFactor{ DkVec4(1.0f) };
        DkVec4 emissiveFactor{ DkVec4(0.0f) };

        int index{ 0 };
        bool unlit{ false };
        bool doubleSided{ false };
        DkF32 emissiveStrength{ 1.0f };
        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

        Ref<Texture2D> baseColorTexture;
        Ref<Texture2D> metallicRoughnessTexture;
        Ref<Texture2D> normalTexture;
        Ref<Texture2D> occlusionTexture;
        Ref<Texture2D> emissiveTexture;

        struct TexCoordSets
        {
            DkU8 baseColor{ 0 };
            DkU8 metallicRoughness{ 0 };
            DkU8 specularGlossiness{ 0 };
            DkU8 normal{ 0 };
            DkU8 occlusion{ 0 };
            DkU8 emissive{ 0 };
        } texCoordSets;

        struct Extension
        {
            Ref<Texture2D> specularGlossinessTexture;
            Ref<Texture2D> diffuseTexture;
            DkVec4 diffuseFactor{ DkVec4(1.0f) };
            DkVec3 specularFactor{ DkVec3(0.0f) };
        } extension;

        struct PbrWorkflows
        {
            bool metallicRoughness{ true };
            bool specularGlossiness{ false };
        } pbrWorkflows;

        std::string name;

        Material() {}
        Material(tinygltf::Material& tinyMaterial, std::vector<Ref<Texture2D>>& textures);

        virtual void Destroy() override {}

        static AssetType GetStaticType() { return AssetType::Material; }
        virtual AssetType GetType() const override { return GetStaticType(); }
    };

    void CreateMaterialBuffer();

}
