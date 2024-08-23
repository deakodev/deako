#include "VulkanMaterial.h"
#include "dkpch.h"

#include "VulkanBase.h"

namespace Deako {

    static Ref<VulkanResources> vr = VulkanBase::GetResources();
    static Ref<VulkanSettings> vs = VulkanBase::GetSettings();

    Material::Material(tinygltf::Material& mat, std::vector<Ref<Texture2D>>& textures)
    {
        doubleSided = mat.doubleSided;

        if (mat.values.find("baseColorTexture") != mat.values.end())
        {
            baseColorTexture = textures[mat.values["baseColorTexture"].TextureIndex()];
            texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
        }

        if (mat.values.find("metallicRoughnessTexture") != mat.values.end())
        {
            metallicRoughnessTexture = textures[mat.values["metallicRoughnessTexture"].TextureIndex()];
            texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
        }

        if (mat.values.find("roughnessFactor") != mat.values.end())
        {
            roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }

        if (mat.values.find("metallicFactor") != mat.values.end())
        {
            metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }

        if (mat.values.find("baseColorFactor") != mat.values.end())
        {
            baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }

        if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end())
        {
            normalTexture = textures[mat.additionalValues["normalTexture"].TextureIndex()];
            texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
        }

        if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end())
        {
            emissiveTexture = textures[mat.additionalValues["emissiveTexture"].TextureIndex()];
            texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
        }

        if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end())
        {
            occlusionTexture = textures[mat.additionalValues["occlusionTexture"].TextureIndex()];
            texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
        }

        if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end())
        {
            tinygltf::Parameter param = mat.additionalValues["alphaMode"];
            if (param.string_value == "BLEND")
                alphaMode = Material::ALPHAMODE_BLEND;

            if (param.string_value == "MASK")
            {
                alphaCutoff = 0.5f;
                alphaMode = Material::ALPHAMODE_MASK;
            }
        }

        if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end())
        {
            alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
        }

        if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end())
        {
            emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
        }

        // extensions
        if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end())
        {
            auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
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

        if (mat.extensions.find("KHR_materials_unlit") != mat.extensions.end())
            unlit = true;

        if (mat.extensions.find("KHR_materials_emissive_strength") != mat.extensions.end())
        {
            auto ext = mat.extensions.find("KHR_materials_emissive_strength");
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

        for (auto& [tag, model] : vr->propModels)
        {
            for (auto& material : model->materials)
            {
                ShaderMaterial shaderMaterial{};

                shaderMaterial.emissiveFactor = material.emissiveFactor;
                // To save space, availabilty and texture coordinate set are combined
                // -1 = texture not used for this material, >= 0 texture used and index of texture coordinate set
                shaderMaterial.colorTextureSet = material.baseColorTexture != nullptr ?
                    material.texCoordSets.baseColor : -1;

                shaderMaterial.normalTextureSet = material.normalTexture != nullptr ?
                    material.texCoordSets.normal : -1;

                shaderMaterial.occlusionTextureSet = material.occlusionTexture != nullptr ?
                    material.texCoordSets.occlusion : -1;

                shaderMaterial.emissiveTextureSet = material.emissiveTexture != nullptr ?
                    material.texCoordSets.emissive : -1;

                shaderMaterial.alphaMask = static_cast<float>(material.alphaMode == Material::ALPHAMODE_MASK);

                shaderMaterial.alphaMaskCutoff = material.alphaCutoff;

                shaderMaterial.emissiveStrength = material.emissiveStrength;

                if (material.pbrWorkflows.metallicRoughness)
                {   // metallic roughness workflow
                    shaderMaterial.workflow = static_cast<float>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
                    shaderMaterial.baseColorFactor = material.baseColorFactor;
                    shaderMaterial.metallicFactor = material.metallicFactor;
                    shaderMaterial.roughnessFactor = material.roughnessFactor;
                    shaderMaterial.PhysicalDescriptorTextureSet = material.metallicRoughnessTexture != nullptr ? material.texCoordSets.metallicRoughness : -1;
                    shaderMaterial.colorTextureSet = material.baseColorTexture != nullptr ? material.texCoordSets.baseColor : -1;
                }
                else if (material.pbrWorkflows.specularGlossiness)
                {   // specular glossiness workflow
                    shaderMaterial.workflow = static_cast<float>(PBR_WORKFLOW_SPECULAR_GLOSSINESS);
                    shaderMaterial.PhysicalDescriptorTextureSet = material.extension.specularGlossinessTexture != nullptr ? material.texCoordSets.specularGlossiness : -1;
                    shaderMaterial.colorTextureSet = material.extension.diffuseTexture != nullptr ? material.texCoordSets.baseColor : -1;
                    shaderMaterial.diffuseFactor = material.extension.diffuseFactor;
                    shaderMaterial.specularFactor = glm::vec4(material.extension.specularFactor, 1.0f);
                }

                shaderMaterials.push_back(shaderMaterial);
            }
        }

        if (vr->shaderMaterialBuffer.buffer != VK_NULL_HANDLE)
            VulkanBuffer::Destroy(vr->shaderMaterialBuffer);

        VkDeviceSize bufferSize = shaderMaterials.size() * sizeof(ShaderMaterial);

        AllocatedBuffer staging = VulkanBuffer::Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vr->device, staging.memory, 0, bufferSize, 0, &staging.mapped));
        memcpy(staging.mapped, shaderMaterials.data(), bufferSize);
        vkUnmapMemory(vr->device, staging.memory);

        // create shader material buffer
        vr->shaderMaterialBuffer = VulkanBuffer::Create(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

        VkBufferCopy copyRegion{ };
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, staging.buffer, vr->shaderMaterialBuffer.buffer, 1, &copyRegion);

        VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

        VulkanBuffer::Destroy(staging);

        // update descriptor
        vr->shaderMaterialDescriptorInfo.buffer = vr->shaderMaterialBuffer.buffer;
        vr->shaderMaterialDescriptorInfo.offset = 0;
        vr->shaderMaterialDescriptorInfo.range = bufferSize;
    }
}
