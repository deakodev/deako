#include "VulkanMaterial.h"
#include "dkpch.h"

#include "Deako/Asset/Pool/ProjectAssetPool.h"

#include "VulkanBase.h"

namespace Deako {

    static Ref<VulkanResources> vr = VulkanBase::GetResources();
    static Ref<VulkanSettings> vs = VulkanBase::GetSettings();

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
            roughnessFactor = static_cast<float>(tinyMaterial.values["roughnessFactor"].Factor());
        }

        if (tinyMaterial.values.find("metallicFactor") != tinyMaterial.values.end())
        {
            metallicFactor = static_cast<float>(tinyMaterial.values["metallicFactor"].Factor());
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
            alphaCutoff = static_cast<float>(tinyMaterial.additionalValues["alphaCutoff"].Factor());
        }

        if (tinyMaterial.additionalValues.find("emissiveFactor") != tinyMaterial.additionalValues.end())
        {
            emissiveFactor = glm::vec4(glm::make_vec3(tinyMaterial.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
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
                for (uint32_t i = 0; i < factor.ArrayLen(); i++)
                {
                    auto val = factor.Get(i);
                    extension.diffuseFactor[i] = val.IsNumber() ?
                        (float)val.Get<double>() : (float)val.Get<int>();
                }
            }
            if (ext->second.Has("specularFactor"))
            {
                auto factor = ext->second.Get("specularFactor");
                for (uint32_t i = 0; i < factor.ArrayLen(); i++)
                {
                    auto val = factor.Get(i);
                    extension.specularFactor[i] = val.IsNumber() ?
                        (float)val.Get<double>() : (float)val.Get<int>();
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
                emissiveStrength = (float)value.Get<double>();
            }
        }
    }

    void CreateMaterialBuffer()
    {
        std::vector<ShaderMaterial> shaderMaterials{};

        uint32_t materialBufferIndex = 0;

        for (Entity entity : vr->entities)
        {
            auto& prefabComp = entity.GetComponent<PrefabComponent>();
            Ref<Model> model = ProjectAssetPool::Get()->GetAsset<Model>(prefabComp.meshHandle);

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

                shaderMaterial.alphaMask = static_cast<float>(material->alphaMode == Material::ALPHAMODE_MASK);

                shaderMaterial.alphaMaskCutoff = material->alphaCutoff;

                shaderMaterial.emissiveStrength = material->emissiveStrength;

                if (material->pbrWorkflows.metallicRoughness)
                {   // metallic roughness workflow
                    shaderMaterial.workflow = static_cast<float>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
                    shaderMaterial.baseColorFactor = material->baseColorFactor;
                    shaderMaterial.metallicFactor = material->metallicFactor;
                    shaderMaterial.roughnessFactor = material->roughnessFactor;
                    shaderMaterial.PhysicalDescriptorTextureSet = material->metallicRoughnessTexture != nullptr ? material->texCoordSets.metallicRoughness : -1;
                    shaderMaterial.colorTextureSet = material->baseColorTexture != nullptr ? material->texCoordSets.baseColor : -1;
                }
                else if (material->pbrWorkflows.specularGlossiness)
                {   // specular glossiness workflow
                    shaderMaterial.workflow = static_cast<float>(PBR_WORKFLOW_SPECULAR_GLOSSINESS);
                    shaderMaterial.PhysicalDescriptorTextureSet = material->extension.specularGlossinessTexture != nullptr ? material->texCoordSets.specularGlossiness : -1;
                    shaderMaterial.colorTextureSet = material->extension.diffuseTexture != nullptr ? material->texCoordSets.baseColor : -1;
                    shaderMaterial.diffuseFactor = material->extension.diffuseFactor;
                    shaderMaterial.specularFactor = glm::vec4(material->extension.specularFactor, 1.0f);
                }

                shaderMaterials.push_back(shaderMaterial);

                material->index = materialBufferIndex;
                materialBufferIndex++;
            }
        }

        if (vr->materialBuffer.buffer.buffer != VK_NULL_HANDLE)
            VulkanBuffer::Destroy(vr->materialBuffer.buffer);

        VkDeviceSize bufferSize = shaderMaterials.size() * sizeof(ShaderMaterial);

        AllocatedBuffer staging = VulkanBuffer::Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vr->device, staging.memory, 0, bufferSize, 0, &staging.mapped));
        memcpy(staging.mapped, shaderMaterials.data(), bufferSize);
        vkUnmapMemory(vr->device, staging.memory);

        // create shader material buffer
        vr->materialBuffer.buffer = VulkanBuffer::Create(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

        VkBufferCopy copyRegion{ };
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, staging.buffer, vr->materialBuffer.buffer.buffer, 1, &copyRegion);

        VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

        VulkanBuffer::Destroy(staging);

        // update descriptor
        vr->materialBuffer.descriptor.buffer = vr->materialBuffer.buffer.buffer;
        vr->materialBuffer.descriptor.offset = 0;
        vr->materialBuffer.descriptor.range = bufferSize;
    }

}
