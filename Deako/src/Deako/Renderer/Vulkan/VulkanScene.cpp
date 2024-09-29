#include "VulkanScene.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"
#include "Deako/Asset/AssetPool.h"

namespace Deako {

    static Ref<VulkanResources> vr = VulkanBase::GetResources();
    static Ref<VulkanSettings> vs = VulkanBase::GetSettings();

    void VulkanScene::Build()
    {
        VulkanBase::Idle();

        SetUpAssets();

        SetUpUniforms();

        SetUpDescriptors();

        SetUpPipelines();

        m_SceneValid = true;
    }

    void VulkanScene::CleanUp()
    {
        VulkanBase::Idle();

        for (auto& [prefix, pipeline] : vr->pipelines)
            vkDestroyPipeline(vr->device, pipeline, nullptr);

        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.scene, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.material, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.node, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.materialBuffer, nullptr);
        vkDestroyDescriptorPool(vr->device, vr->descriptorPool, nullptr);

        for (auto& uniform : vr->uniforms)
        {
            VulkanBuffer::Destroy(uniform.dynamic.buffer);
            VulkanBuffer::Destroy(uniform.shared.buffer);
            VulkanBuffer::Destroy(uniform.params.buffer);
        }

        VulkanBuffer::Destroy(vr->shaderMaterialBuffer);

        vr->textures.lutBrdf->Destroy();
        vr->skybox.irradianceCube->Destroy();
        vr->skybox.prefilteredCube->Destroy();

        vr->entities.clear();
    }

    void VulkanScene::Rebuild()
    {
        CleanUp();
        Build();
    }

    void VulkanScene::SetUpAssets()
    {
        if (!vr->textures.empty)
            vr->textures.empty = AssetPool::Import<Texture2D>("textures/empty.ktx");

        vr->entities = Scene::GetAllEntitiesWith<TagComponent, ModelComponent>();

        for (auto it = vr->entities.begin(); it != vr->entities.end(); )
        {
            Entity entity = *it;
            std::string& tag = entity.GetComponent<TagComponent>().tag;

            if (tag == "Skybox")
            {
                auto& modelComp = entity.GetComponent<ModelComponent>();
                Ref<Model> model = AssetPool::GetAsset<Model>(modelComp.handle);

                // easy access, separate from others
                vr->skybox.model = model;
                // erase the entity and update the iterator
                vr->entities.erase(it);

                // Handle the skybox-specific components
                auto& textureComp = entity.GetComponent<TextureComponent>();
                vr->skybox.environmentCube = AssetPool::GetAsset<TextureCubeMap>(textureComp.handle);
                vr->skybox.irradianceCube->GenerateCubeMap();
                vr->skybox.prefilteredCube->GenerateCubeMap();
                break;
            }

            ++it;
        }

        CreateMaterialBuffer();
        GenerateBRDFLookUpTable();
    }

    void VulkanScene::SetUpUniforms()
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vr->physicalDevice, &deviceProperties);

        // determine required alignment based on min device offset alignment
        size_t minUniformAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        vr->dynamicUniformAlignment = sizeof(glm::mat4);

        if (minUniformAlignment > 0)
            vr->dynamicUniformAlignment = (vr->dynamicUniformAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

        size_t dynamicBufferSize = vr->entities.size() * vr->dynamicUniformAlignment;

        vr->uniformDataDynamic.model = (glm::mat4*)VulkanMemory::AlignedAlloc(dynamicBufferSize, vr->dynamicUniformAlignment);
        DK_CORE_ASSERT(vr->uniformDataDynamic.model);

        VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        vr->uniforms.resize(vr->swapchain.imageCount);
        for (auto& uniform : vr->uniforms)
        {
            // dynamic uniform buffer object with model matrix
            uniform.dynamic.buffer = VulkanBuffer::Create(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniform.dynamic.descriptor = { uniform.dynamic.buffer.buffer, 0, vr->dynamicUniformAlignment };
            VkCR(vkMapMemory(vr->device, uniform.dynamic.buffer.memory, 0, dynamicBufferSize, 0, &uniform.dynamic.buffer.mapped));

            // shared uniform buffer object with projection and view matrix
            uniform.shared.buffer = VulkanBuffer::Create(sizeof(vr->uniformDataShared), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.shared.descriptor = { uniform.shared.buffer.buffer, 0, sizeof(vr->uniformDataShared) };
            VkCR(vkMapMemory(vr->device, uniform.shared.buffer.memory, 0, sizeof(vr->uniformDataShared), 0, &uniform.shared.buffer.mapped));

            // shared uniform buffer object with light params
            uniform.params.buffer = VulkanBuffer::Create(sizeof(vr->shaderValuesParams), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.params.descriptor = { uniform.params.buffer.buffer, 0, sizeof(vr->shaderValuesParams) };
            VkCR(vkMapMemory(vr->device, uniform.params.buffer.memory, 0, sizeof(vr->shaderValuesParams), 0, &uniform.params.buffer.mapped));
        }

        UpdateUniforms();
    }

    void VulkanScene::UpdateUniforms()
    {
        // shared scene
        vr->uniformDataShared.projection = vr->camera.matrices.perspective;
        vr->uniformDataShared.view = vr->camera.matrices.view;

        glm::mat4 cv = glm::inverse(vr->camera.matrices.view);
        vr->uniformDataShared.camPos = glm::vec3(cv[3]);

        // models
        uint32_t index = 0;
        for (Entity entity : vr->entities)
        {
            // aligned offset for dynamic uniform
            glm::mat4* modelMatrix = (glm::mat4*)(((uint64_t)vr->uniformDataDynamic.model + (index * vr->dynamicUniformAlignment)));

            *modelMatrix = entity.GetComponent<TransformComponent>().GetTransform();

            // auto& modelComp = entity.GetComponent<ModelComponent>();
            // glm::mat4 aabb = AssetPool::GetAsset<Model>(modelComp.handle)->aaBoundingBox;

            // float scaleX = glm::length(glm::vec3(aabb[0]));
            // float scaleY = glm::length(glm::vec3(aabb[1]));
            // float scaleZ = glm::length(glm::vec3(aabb[2]));
            // float scaleFactor = (1.0f / std::max(scaleX, std::max(scaleY, scaleZ))) * 6.0f;

            // glm::vec3 aabbMin = glm::vec3(aabb[3][0], aabb[3][1], aabb[3][2]);
            // glm::vec3 aabbMax = aabbMin + glm::vec3(scaleX, scaleY, scaleZ);
            // glm::vec3 centroid = (aabbMin + aabbMax) / 2.0f; // center of the models aabb
            // glm::vec3 translate = -centroid;

            // static auto startTime = std::chrono::high_resolution_clock::now();
            // auto currentTime = std::chrono::high_resolution_clock::now();
            // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            index++;
        }

        VulkanResources::UniformSet uniformSet = vr->uniforms[vr->currentFrame];
        memcpy(uniformSet.dynamic.buffer.mapped, vr->uniformDataDynamic.model, vr->dynamicUniformAlignment * vr->entities.size());
        memcpy(uniformSet.shared.buffer.mapped, &vr->uniformDataShared, sizeof(vr->uniformDataShared));
        memcpy(uniformSet.params.buffer.mapped, &vr->shaderValuesParams, sizeof(vr->shaderValuesParams));

        // flush dynamic uniform to make changes visible to the host
        VkMappedMemoryRange memoryRange{};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = uniformSet.dynamic.buffer.memory;
        memoryRange.size = vr->dynamicUniformAlignment * vr->entities.size();
        vkFlushMappedMemoryRanges(vr->device, 1, &memoryRange);
    }

    void VulkanScene::SetUpDescriptors()
    {
        /* DESCRIPTOR POOL */
        uint32_t imageSamplerCount = 0;
        uint32_t materialCount = 0;
        uint32_t meshCount = 0;

        // environment samplers (radiance, irradiance, brdf lut)
        imageSamplerCount += 3;

        for (Entity entity : vr->entities)
        {
            auto& materialHandles = entity.GetComponent<MaterialComponent>().handles;
            uint32_t entityMaterialCount = materialHandles.size();

            imageSamplerCount += (entityMaterialCount * 5);
            materialCount += entityMaterialCount;

            auto modelHandle = entity.GetComponent<ModelComponent>().handle;
            Ref<Model> model = AssetPool::GetAsset<Model>(modelHandle);

            for (auto node : model->linearNodes)
                if (node->mesh) meshCount++;
        }

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * vr->swapchain.imageCount },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, (4 + meshCount) * vr->swapchain.imageCount },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * vr->swapchain.imageCount },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 } // One SSBO for the shader material buffer
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = (3 + materialCount + meshCount) * vr->swapchain.imageCount;
        VkCR(vkCreateDescriptorPool(vr->device, &poolInfo, nullptr, &vr->descriptorPool));

        /* DESCRIPTOR SETS */
        vr->descriptorSets.resize(vr->swapchain.imageCount);

        {   // scene (matrices and environment maps)
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
            };

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pBindings = setLayoutBindings.data();
            layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
            VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.scene));

            for (auto i = 0; i < vr->descriptorSets.size(); i++)
            {
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = vr->descriptorPool;
                allocInfo.pSetLayouts = &vr->descriptorSetLayouts.scene;
                allocInfo.descriptorSetCount = 1;
                VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &vr->descriptorSets[i].scene));

                std::array<VkWriteDescriptorSet, 6> write{};

                write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[0].descriptorCount = 1;
                write[0].dstSet = vr->descriptorSets[i].scene;
                write[0].dstBinding = 0;
                write[0].pBufferInfo = &vr->uniforms[i].shared.descriptor;

                write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[1].descriptorCount = 1;
                write[1].dstSet = vr->descriptorSets[i].scene;
                write[1].dstBinding = 1;
                write[1].pBufferInfo = &vr->uniforms[i].params.descriptor;

                write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[2].descriptorCount = 1;
                write[2].dstSet = vr->descriptorSets[i].scene;
                write[2].dstBinding = 2;
                write[2].pImageInfo = &vr->skybox.irradianceCube->descriptor;

                write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[3].descriptorCount = 1;
                write[3].dstSet = vr->descriptorSets[i].scene;
                write[3].dstBinding = 3;
                write[3].pImageInfo = &vr->skybox.prefilteredCube->descriptor;

                write[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[4].descriptorCount = 1;
                write[4].dstSet = vr->descriptorSets[i].scene;
                write[4].dstBinding = 4;
                write[4].pImageInfo = &vr->textures.lutBrdf->descriptor;

                write[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                write[5].descriptorCount = 1;
                write[5].dstSet = vr->descriptorSets[i].scene;
                write[5].dstBinding = 5;
                write[5].pBufferInfo = &vr->uniforms[i].dynamic.descriptor;


                vkUpdateDescriptorSets(vr->device, static_cast<uint32_t>(write.size()), write.data(), 0, NULL);
            }
        }

        {   // material (samplers)
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
            };

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pBindings = setLayoutBindings.data();
            layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
            VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.material));

            // per-material descriptor sets
            for (Entity entity : vr->entities)
            {
                auto& modelComp = entity.GetComponent<ModelComponent>();
                Ref<Model> model = AssetPool::GetAsset<Model>(modelComp.handle);

                for (auto& material : model->materials)
                {
                    VkDescriptorImageInfo emptyTextureDescriptor = vr->textures.empty->descriptor;

                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = vr->descriptorPool;
                    allocInfo.pSetLayouts = &vr->descriptorSetLayouts.material;
                    allocInfo.descriptorSetCount = 1;
                    VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &material->descriptorSet));

                    std::vector<VkDescriptorImageInfo> imageDescriptors = {
                        emptyTextureDescriptor,
                        emptyTextureDescriptor,
                        material->normalTexture ? material->normalTexture->descriptor : emptyTextureDescriptor,
                        material->occlusionTexture ? material->occlusionTexture->descriptor : emptyTextureDescriptor,
                        material->emissiveTexture ? material->emissiveTexture->descriptor : emptyTextureDescriptor
                    };

                    if (material->pbrWorkflows.metallicRoughness)
                    {
                        if (material->baseColorTexture)
                            imageDescriptors[0] = material->baseColorTexture->descriptor;
                        if (material->metallicRoughnessTexture)
                            imageDescriptors[1] = material->metallicRoughnessTexture->descriptor;
                    }
                    else if (material->pbrWorkflows.specularGlossiness)
                    {
                        if (material->extension.diffuseTexture)
                            imageDescriptors[0] = material->extension.diffuseTexture->descriptor;
                        if (material->extension.specularGlossinessTexture)
                            imageDescriptors[1] = material->extension.specularGlossinessTexture->descriptor;
                    }

                    std::array<VkWriteDescriptorSet, 5> write{};
                    for (size_t i = 0; i < imageDescriptors.size(); i++)
                    {
                        write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write[i].descriptorCount = 1;
                        write[i].dstSet = material->descriptorSet;
                        write[i].dstBinding = static_cast<uint32_t>(i);
                        write[i].pImageInfo = &imageDescriptors[i];
                    }

                    vkUpdateDescriptorSets(vr->device, static_cast<uint32_t>(write.size()), write.data(), 0, NULL);
                }
            }

            {   // model node (matrices)
                std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
                };

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.pBindings = setLayoutBindings.data();
                layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
                VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.node));

                for (Entity entity : vr->entities)
                {
                    auto& modelComp = entity.GetComponent<ModelComponent>();
                    Ref<Model> model = AssetPool::GetAsset<Model>(modelComp.handle);

                    for (auto& node : model->nodes) // per-node descriptor set
                        node->SetDescriptorSet();
                }
            }

            {   // material buffer
                std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                    { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                };
                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.pBindings = setLayoutBindings.data();
                layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
                VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.materialBuffer));

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = vr->descriptorPool;
                allocInfo.pSetLayouts = &vr->descriptorSetLayouts.materialBuffer;
                allocInfo.descriptorSetCount = 1;
                VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &vr->shaderMaterialDescriptorSet));

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                write.descriptorCount = 1;
                write.dstSet = vr->shaderMaterialDescriptorSet;
                write.dstBinding = 0;
                write.pBufferInfo = &vr->shaderMaterialDescriptorInfo;
                vkUpdateDescriptorSets(vr->device, 1, &write, 0, nullptr);
            }
        }

        {   // skybox (fixed set)
            for (auto i = 0; i < vr->uniforms.size(); i++)
            {
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = vr->descriptorPool;
                allocInfo.pSetLayouts = &vr->descriptorSetLayouts.scene;
                allocInfo.descriptorSetCount = 1;
                VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &vr->descriptorSets[i].skybox));

                std::array<VkWriteDescriptorSet, 3> write{};
                write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[0].descriptorCount = 1;
                write[0].dstSet = vr->descriptorSets[i].skybox;
                write[0].dstBinding = 0;
                write[0].pBufferInfo = &vr->uniforms[i].shared.descriptor;

                write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[1].descriptorCount = 1;
                write[1].dstSet = vr->descriptorSets[i].skybox;
                write[1].dstBinding = 1;
                write[1].pBufferInfo = &vr->uniforms[i].params.descriptor;

                write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[2].descriptorCount = 1;
                write[2].dstSet = vr->descriptorSets[i].skybox;
                write[2].dstBinding = 2;
                write[2].pImageInfo = &vr->skybox.prefilteredCube->descriptor;

                vkUpdateDescriptorSets(vr->device, static_cast<uint32_t>(write.size()), write.data(), 0, nullptr);
            }
        }
    }

    void VulkanScene::SetUpPipelines()
    {
        // depending on material setting, we need different pipeline variants per set
        // eg. one with back-face culling, one without and one with alpha-blending enabled
        AddPipelineSet("skybox", "shaders/bin/skybox.vert.spv", "shaders/bin/skybox.frag.spv");

        AddPipelineSet("pbr", "shaders/bin/pbr.vert.spv", "shaders/bin/material_pbr.frag.spv");

        AddPipelineSet("unlit", "shaders/bin/pbr.vert.spv", "shaders/bin/material_unlit.frag.spv");
    }

    void VulkanScene::AddPipelineSet(const std::string prefix, std::filesystem::path vertexShader, std::filesystem::path fragmentShader)
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterization{};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &blendAttachment;
        colorBlend.pNext = nullptr;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = (prefix == "skybox" ? VK_FALSE : VK_TRUE);
        depthStencil.depthWriteEnable = (prefix == "skybox" ? VK_FALSE : VK_TRUE);
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.front = depthStencil.back;
        depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

        VkPipelineViewportStateCreateInfo viewport{};
        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport.viewportCount = 1;
        viewport.scissorCount = 1;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        if (vs->multiSampling)
            multisample.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

        std::vector<VkDynamicState> dynamicEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.pDynamicStates = dynamicEnables.data();
        dynamic.dynamicStateCount = static_cast<uint32_t>(dynamicEnables.size());

        std::vector<VkDescriptorSetLayout> setLayouts = {
                vr->descriptorSetLayouts.scene,
                vr->descriptorSetLayouts.material,
                vr->descriptorSetLayouts.node,
                vr->descriptorSetLayouts.materialBuffer,
        };

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.size = sizeof(uint32_t);
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        if (vr->pipelineLayout == VK_NULL_HANDLE)
        {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
            pipelineLayoutInfo.pSetLayouts = setLayouts.data();
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            VkCR(vkCreatePipelineLayout(vr->device, &pipelineLayoutInfo, nullptr, &vr->pipelineLayout));
        }

        // vertex bindings and attributes
        VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

        std::vector<VkVertexInputAttributeDescription>  vertexInputAttributes;

        if (prefix == "skybox")
        {
            vertexInputAttributes = {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos)},
                { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal) },
                { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv0) },
            };
        }
        else
        {
            vertexInputAttributes = {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos)},
                { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal) },
                { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv0) },
                { 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv1) },
                { 4, 0, VK_FORMAT_R32G32B32A32_UINT, offsetof(Model::Vertex, joint0) },
                { 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, weight0) },
                { 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, color) }
            };
        }

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &vertexInputBinding;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInput.pVertexAttributeDescriptions = vertexInputAttributes.data();

        // shaders
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].pName = "main";
        shaderStages[0].module = VulkanShader::CreateShaderModule(vertexShader);

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].pName = "main";
        shaderStages[1].module = VulkanShader::CreateShaderModule(fragmentShader);

        // pipeline rendering info
        VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &vr->multisampleTarget.color.format;
        pipelineRenderingCreateInfo.depthAttachmentFormat = vr->multisampleTarget.depth.format;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = vr->multisampleTarget.depth.format;

        // pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = vr->pipelineLayout;
        // pipelineInfo.renderPass = vr->renderPass;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pRasterizationState = &rasterization;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pViewportState = &viewport;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamic;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pNext = &pipelineRenderingCreateInfo;

        VkPipeline pipeline = {};

        // default pipeline with back-face culling
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
        vr->pipelines[prefix] = pipeline;

        // double sided
        rasterization.cullMode = VK_CULL_MODE_NONE;
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
        vr->pipelines[prefix + "_double_sided"] = pipeline;

        // alpha blending
        rasterization.cullMode = VK_CULL_MODE_NONE;
        blendAttachment.blendEnable = VK_TRUE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
        vr->pipelines[prefix + "_alpha_blending"] = pipeline;

        for (auto shaderStage : shaderStages)
            vkDestroyShaderModule(vr->device, shaderStage.module, nullptr);
    }

    void VulkanScene::Draw(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        AllocatedImage& colorTarget = vr->multisampleTarget.color;
        AllocatedImage& depthTarget = vr->multisampleTarget.depth;
        AllocatedImage& viewportImage = vr->viewport.images[imageIndex];

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        colorAttachment.resolveImageView = viewportImage.view;
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingAttachmentInfo depthStencilAttachment = {};
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthStencilAttachment.pNext = nullptr;
        depthStencilAttachment.imageView = depthTarget.view;
        depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, colorTarget.extent.width, colorTarget.extent.height };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = &depthStencilAttachment;
        renderInfo.pStencilAttachment = &depthStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        VkViewport viewport{};
        viewport.width = colorTarget.extent.width;
        viewport.height = colorTarget.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = { colorTarget.extent.width,  colorTarget.extent.height };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        uint32_t dynamicOffset = 0;

        if (vs->displayBackground)
        {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->pipelineLayout, 0, 1, &vr->descriptorSets[vr->currentFrame].skybox, 1, &dynamicOffset);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->pipelines["skybox"]);

            vr->skybox.model->Draw(commandBuffer);
        }

        VkDeviceSize offsets[1] = { 0 };

        uint32_t index = 0;
        for (Entity entity : vr->entities)
        {
            auto& modelComp = entity.GetComponent<ModelComponent>();
            Ref<Model> model = AssetPool::GetAsset<Model>(modelComp.handle);

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertices.buffer, offsets);

            if (model->indices.buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(commandBuffer, model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            vr->boundPipeline = VK_NULL_HANDLE;

            // one dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
            dynamicOffset = index * static_cast<uint32_t>(vr->dynamicUniformAlignment);

            // opaque primitives first
            for (auto node : model->nodes)
                RenderNode(node, commandBuffer, Material::ALPHAMODE_OPAQUE, dynamicOffset);
            // alpha masked primitives
            for (auto node : model->nodes)
                RenderNode(node, commandBuffer, Material::ALPHAMODE_MASK, dynamicOffset);
            // transparent primitives
            for (auto node : model->nodes) // TODO: Correct depth sorting
                RenderNode(node, commandBuffer, Material::ALPHAMODE_BLEND, dynamicOffset);

            index++;
        }

        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanScene::UpdateShaderParams()
    {
        vr->shaderValuesParams.lightDir = glm::vec4(
            sin(glm::radians(vr->lightSource.rotation.x)) * cos(glm::radians(vr->lightSource.rotation.y)),
            sin(glm::radians(vr->lightSource.rotation.y)),
            cos(glm::radians(vr->lightSource.rotation.x)) * cos(glm::radians(vr->lightSource.rotation.y)),
            0.0f);
    }

    void VulkanScene::ViewportResize(const glm::vec2& viewportSize)
    {
        vr->camera.updateAspectRatio((float)viewportSize.x / (float)viewportSize.y);
    }

}
