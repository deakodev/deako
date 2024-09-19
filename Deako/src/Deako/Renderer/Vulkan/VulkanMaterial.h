#pragma once

#include "VulkanTypes.h"
#include "VulkanTexture.h"
#include "Deako/Asset/Asset.h"

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Deako {

    enum PBRWorkflows { PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSSINESS = 1 };

    struct alignas(16) ShaderMaterial
    {
        glm::vec4 baseColorFactor;
        glm::vec4 emissiveFactor;
        glm::vec4 diffuseFactor;
        glm::vec4 specularFactor;
        float workflow;
        int colorTextureSet;
        int PhysicalDescriptorTextureSet;
        int normalTextureSet;
        int occlusionTextureSet;
        int emissiveTextureSet;
        float metallicFactor;
        float roughnessFactor;
        float alphaMask;
        float alphaMaskCutoff;
        float emissiveStrength;
    };

    struct Material : public Asset
    {
        enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
        AlphaMode alphaMode{ ALPHAMODE_OPAQUE };

        float alphaCutoff{ 1.0f };
        float metallicFactor{ 1.0f };
        float roughnessFactor{ 1.0f };
        glm::vec4 baseColorFactor{ glm::vec4(1.0f) };
        glm::vec4 emissiveFactor{ glm::vec4(0.0f) };

        int index{ 0 };
        bool unlit{ false };
        bool doubleSided{ false };
        float emissiveStrength{ 1.0f };
        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

        Ref<Texture2D> baseColorTexture;
        Ref<Texture2D> metallicRoughnessTexture;
        Ref<Texture2D> normalTexture;
        Ref<Texture2D> occlusionTexture;
        Ref<Texture2D> emissiveTexture;

        struct TexCoordSets
        {
            uint8_t baseColor{ 0 };
            uint8_t metallicRoughness{ 0 };
            uint8_t specularGlossiness{ 0 };
            uint8_t normal{ 0 };
            uint8_t occlusion{ 0 };
            uint8_t emissive{ 0 };
        } texCoordSets;

        struct Extension
        {
            Ref<Texture2D> specularGlossinessTexture;
            Ref<Texture2D> diffuseTexture;
            glm::vec4 diffuseFactor{ glm::vec4(1.0f) };
            glm::vec3 specularFactor{ glm::vec3(0.0f) };
        } extension;

        struct PbrWorkflows
        {
            bool metallicRoughness{ true };
            bool specularGlossiness{ false };
        } pbrWorkflows;

        Material() {}
        Material(tinygltf::Material& tinyMaterial, std::vector<Ref<Texture2D>>& textures);

        virtual void Destroy() override {}

        static AssetType GetStaticType() { return AssetType::Material; }
        virtual AssetType GetType() const override { return GetStaticType(); }
    };

    void CreateMaterialBuffer();

}
