#include "VulkanMaterial.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanScene.h"
#include "VulkanResource.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Scene/Entity.h"

namespace Deako {

    Material::Material(tinygltf::Material& tinyMaterial, std::vector<Ref<Texture2D>>& textures)
    {
        name = tinyMaterial.name;
        doubleSided = tinyMaterial.doubleSided;

        if (tinyMaterial.values.find("baseColorTexture") != tinyMaterial.values.end())
        {
            baseColorTexture = textures[tinyMaterial.values["baseColorTexture"].TextureIndex()];
            texCoordSets.baseColor = tinyMaterial.values["baseColorTexture"].TextureTexCoord();
        }

        if (tinyMaterial.values.find("metallicRoughnessTexture") != tinyMaterial.values.end())
        {
            metallicRoughnessTexture = textures[tinyMaterial.values["metallicRoughnessTexture"].TextureIndex()];
            texCoordSets.metallicRoughness = tinyMaterial.values["metallicRoughnessTexture"].TextureTexCoord();
        }

        if (tinyMaterial.values.find("roughnessFactor") != tinyMaterial.values.end())
        {
            roughnessFactor = static_cast<DkF32>(tinyMaterial.values["roughnessFactor"].Factor());
        }

        if (tinyMaterial.values.find("metallicFactor") != tinyMaterial.values.end())
        {
            metallicFactor = static_cast<DkF32>(tinyMaterial.values["metallicFactor"].Factor());
        }

        if (tinyMaterial.values.find("baseColorFactor") != tinyMaterial.values.end())
        {
            baseColorFactor = glm::make_vec4(tinyMaterial.values["baseColorFactor"].ColorFactor().data());
        }

        if (tinyMaterial.additionalValues.find("normalTexture") != tinyMaterial.additionalValues.end())
        {
            normalTexture = textures[tinyMaterial.additionalValues["normalTexture"].TextureIndex()];
            texCoordSets.normal = tinyMaterial.additionalValues["normalTexture"].TextureTexCoord();
        }

        if (tinyMaterial.additionalValues.find("emissiveTexture") != tinyMaterial.additionalValues.end())
        {
            emissiveTexture = textures[tinyMaterial.additionalValues["emissiveTexture"].TextureIndex()];
            texCoordSets.emissive = tinyMaterial.additionalValues["emissiveTexture"].TextureTexCoord();
        }

        if (tinyMaterial.additionalValues.find("occlusionTexture") != tinyMaterial.additionalValues.end())
        {
            occlusionTexture = textures[tinyMaterial.additionalValues["occlusionTexture"].TextureIndex()];
            texCoordSets.occlusion = tinyMaterial.additionalValues["occlusionTexture"].TextureTexCoord();
        }

        if (tinyMaterial.additionalValues.find("alphaMode") != tinyMaterial.additionalValues.end())
        {
            tinygltf::Parameter param = tinyMaterial.additionalValues["alphaMode"];
            if (param.string_value == "BLEND")
                alphaMode = Material::ALPHAMODE_BLEND;

            if (param.string_value == "MASK")
            {
                alphaCutoff = 0.5f;
                alphaMode = Material::ALPHAMODE_MASK;
            }
        }

        if (tinyMaterial.additionalValues.find("alphaCutoff") != tinyMaterial.additionalValues.end())
        {
            alphaCutoff = static_cast<DkF32>(tinyMaterial.additionalValues["alphaCutoff"].Factor());
        }

        if (tinyMaterial.additionalValues.find("emissiveFactor") != tinyMaterial.additionalValues.end())
        {
            emissiveFactor = DkVec4(glm::make_vec3(tinyMaterial.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
        }

        // extensions
        if (tinyMaterial.extensions.find("KHR_materials_pbrSpecularGlossiness") != tinyMaterial.extensions.end())
        {
            auto ext = tinyMaterial.extensions.find("KHR_materials_pbrSpecularGlossiness");
            if (ext->second.Has("specularGlossinessTexture"))
            {
                auto index = ext->second.Get("specularGlossinessTexture").Get("index");
                extension.specularGlossinessTexture = textures[index.Get<int>()];
                auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
                texCoordSets.specularGlossiness = texCoordSet.Get<int>();
                pbrWorkflows.specularGlossiness = true;
                pbrWorkflows.metallicRoughness = false;
            }
            if (ext->second.Has("diffuseTexture"))
            {
                auto index = ext->second.Get("diffuseTexture").Get("index");
                extension.diffuseTexture = textures[index.Get<int>()];
            }
            if (ext->second.Has("diffuseFactor"))
            {
                auto factor = ext->second.Get("diffuseFactor");
                for (DkU32 i = 0; i < factor.ArrayLen(); i++)
                {
                    auto val = factor.Get(i);
                    extension.diffuseFactor[i] = val.IsNumber() ?
                        (DkF32)val.Get<DkF64>() : (DkF32)val.Get<int>();
                }
            }
            if (ext->second.Has("specularFactor"))
            {
                auto factor = ext->second.Get("specularFactor");
                for (DkU32 i = 0; i < factor.ArrayLen(); i++)
                {
                    auto val = factor.Get(i);
                    extension.specularFactor[i] = val.IsNumber() ?
                        (DkF32)val.Get<DkF64>() : (DkF32)val.Get<int>();
                }
            }
        }

        if (tinyMaterial.extensions.find("KHR_materials_unlit") != tinyMaterial.extensions.end())
            unlit = true;

        if (tinyMaterial.extensions.find("KHR_materials_emissive_strength") != tinyMaterial.extensions.end())
        {
            auto ext = tinyMaterial.extensions.find("KHR_materials_emissive_strength");
            if (ext->second.Has("emissiveStrength"))
            {
                auto value = ext->second.Get("emissiveStrength");
                emissiveStrength = (DkF32)value.Get<DkF64>();
            }
        }
    }

    void CreateMaterialBuffer()
    {
        ProjectAssetPool& projectAssetPool = Deako::GetProjectAssetPool();
        Scene& activeScene = Deako::GetActiveScene();
        VulkanBaseResources& vb = VulkanBase::GetResources();
        VulkanSceneResources& vs = activeScene.GetVulkanScene().GetResources();

        std::vector<ShaderMaterial> shaderMaterials{};

        DkU32 materialBufferIndex = 0;

        for (auto& entity : activeScene.GetEntities())
        {
            if (entity.HasComponent<PrefabComponent>())
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> model = projectAssetPool.GetAsset<Model>(prefabComp.meshHandle);

                for (auto& material : model->materials)
                {
                    ShaderMaterial shaderMaterial{};

                    shaderMaterial.emissiveFactor = material->emissiveFactor;
                    // To save space, availabilty and texture coordinate set are combined
                    // -1 = texture not used for this material, >= 0 texture used and index of texture coordinate set
                    shaderMaterial.colorTextureSet = material->baseColorTexture != nullptr ?
                        material->texCoordSets.baseColor : -1;

                    shaderMaterial.normalTextureSet = material->normalTexture != nullptr ?
                        material->texCoordSets.normal : -1;

                    shaderMaterial.occlusionTextureSet = material->occlusionTexture != nullptr ?
                        material->texCoordSets.occlusion : -1;

                    shaderMaterial.emissiveTextureSet = material->emissiveTexture != nullptr ?
                        material->texCoordSets.emissive : -1;

                    shaderMaterial.alphaMask = static_cast<DkF32>(material->alphaMode == Material::ALPHAMODE_MASK);

                    shaderMaterial.alphaMaskCutoff = material->alphaCutoff;

                    shaderMaterial.emissiveStrength = material->emissiveStrength;

                    if (material->pbrWorkflows.metallicRoughness)
                    {   // metallic roughness workflow
                        shaderMaterial.workflow = static_cast<DkF32>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
                        shaderMaterial.baseColorFactor = material->baseColorFactor;
                        shaderMaterial.metallicFactor = material->metallicFactor;
                        shaderMaterial.roughnessFactor = material->roughnessFactor;
                        shaderMaterial.PhysicalDescriptorTextureSet = material->metallicRoughnessTexture != nullptr ? material->texCoordSets.metallicRoughness : -1;
                        shaderMaterial.colorTextureSet = material->baseColorTexture != nullptr ? material->texCoordSets.baseColor : -1;
                    }
                    else if (material->pbrWorkflows.specularGlossiness)
                    {   // specular glossiness workflow
                        shaderMaterial.workflow = static_cast<DkF32>(PBR_WORKFLOW_SPECULAR_GLOSSINESS);
                        shaderMaterial.PhysicalDescriptorTextureSet = material->extension.specularGlossinessTexture != nullptr ? material->texCoordSets.specularGlossiness : -1;
                        shaderMaterial.colorTextureSet = material->extension.diffuseTexture != nullptr ? material->texCoordSets.baseColor : -1;
                        shaderMaterial.diffuseFactor = material->extension.diffuseFactor;
                        shaderMaterial.specularFactor = DkVec4(material->extension.specularFactor, 1.0f);
                    }

                    shaderMaterials.push_back(shaderMaterial);

                    material->index = materialBufferIndex;
                    materialBufferIndex++;
                }
            }
        }

        if (vs.materialBuffer.buffer.buffer != VK_NULL_HANDLE)
            VulkanBuffer::Destroy(vs.materialBuffer.buffer);

        VkDeviceSize bufferSize = shaderMaterials.size() * sizeof(ShaderMaterial);

        AllocatedBuffer staging = VulkanBuffer::Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vb.device, staging.memory, 0, bufferSize, 0, &staging.mapped));
        memcpy(staging.mapped, shaderMaterials.data(), bufferSize);
        vkUnmapMemory(vb.device, staging.memory);

        // create shader material buffer
        vs.materialBuffer.buffer = VulkanBuffer::Create(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vb.singleUseCommandPool);

        VkBufferCopy copyRegion{ };
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, staging.buffer, vs.materialBuffer.buffer.buffer, 1, &copyRegion);

        VulkanCommand::EndSingleTimeCommands(vb.singleUseCommandPool, commandBuffer);

        VulkanBuffer::Destroy(staging);

        // update descriptor
        vs.materialBuffer.descriptor.buffer = vs.materialBuffer.buffer.buffer;
        vs.materialBuffer.descriptor.offset = 0;
        vs.materialBuffer.descriptor.range = bufferSize;
    }

}
