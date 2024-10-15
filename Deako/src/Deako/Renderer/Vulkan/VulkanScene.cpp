#include "VulkanScene.h"
#include "dkpch.h"

#include "Deako/Asset/Scene/SceneHandler.h"
#include "Deako/Asset/Texture/TextureHandler.h"

#include "VulkanBase.h"
#include "VulkanPipeline.h"

namespace Deako {

    static Ref<VulkanBaseResources> vbr = VulkanBase::GetResources();
    static Ref<VulkanBaseContext> vbc = VulkanBase::GetContext();
    static Ref<VulkanBaseSettings> vbs = VulkanBase::GetSettings();

    void VulkanScene::Build()
    {
        VulkanBase::Idle();

        SetUpAssets();

        SetUpUniforms();

        SetUpDescriptors();

        SetUpPipelines();

        vsc->sceneValid = true;
    }

    void VulkanScene::CleanUp()
    {
        VulkanBase::Idle();

        vkDestroyPipeline(vbr->device, vsr->pipelines.skybox, nullptr);
        vkDestroyPipeline(vbr->device, vsr->pipelines.pbr, nullptr);
        vkDestroyPipeline(vbr->device, vsr->pipelines.pbrDoubleSided, nullptr);
        vkDestroyPipeline(vbr->device, vsr->pipelines.pbrAlphaBlending, nullptr);
        vkDestroyPipeline(vbr->device, vsr->pipelines.unlit, nullptr);
        vkDestroyPipeline(vbr->device, vsr->pipelines.unlitDoubleSided, nullptr);
        vkDestroyPipeline(vbr->device, vsr->pipelines.unlitAlphaBlending, nullptr);

        vkDestroyPipelineLayout(vbr->device, vsr->scenePipelineLayout, nullptr);
        vkDestroyPipelineLayout(vbr->device, vsr->skyboxPipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(vbr->device, vsr->descriptorLayouts.scene, nullptr);
        vkDestroyDescriptorSetLayout(vbr->device, vsr->descriptorLayouts.skybox, nullptr);
        vkDestroyDescriptorSetLayout(vbr->device, vsr->descriptorLayouts.material, nullptr);
        vkDestroyDescriptorSetLayout(vbr->device, vsr->descriptorLayouts.node, nullptr);
        vkDestroyDescriptorSetLayout(vbr->device, vsr->descriptorLayouts.materialBuffer, nullptr);

        for (auto& frame : vbc->frames)
            frame.descriptorAllocator.DestroyPools();

        vsr->staticDescriptorAllocator.DestroyPools();

        for (auto& uniform : vsr->uniforms)
        {
            VulkanBuffer::Destroy(uniform.dynamic.buffer);
            VulkanBuffer::Destroy(uniform.shared.buffer);
            VulkanBuffer::Destroy(uniform.light.buffer);
        }

        VulkanBuffer::Destroy(vsr->materialBuffer.buffer);

        vsr->lightSource.lutBrdf->Destroy();
        vsr->skybox.irradianceCube->Destroy();
        vsr->skybox.prefilteredCube->Destroy();

        vsc->entities.clear();
    }

    void VulkanScene::Rebuild()
    {
        CleanUp();
        Build();
    }

    void VulkanScene::SetUpAssets()
    {
        vsc->projectAssetPool = ProjectAssetPool::Get();

        Ref<Scene> scene = SceneHandler::GetActiveScene();
        vsc->entities = scene->GetAllEntitiesWith<PrefabComponent>();

        for (auto it = vsc->entities.begin(); it != vsc->entities.end(); )
        {
            Entity entity = *it;
            std::string& tag = entity.GetComponent<TagComponent>().tag;

            if (tag == "Skybox")
            {
                vss->displayBackground = true;

                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> mesh = vsc->projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

                vsr->skybox.model = mesh;  // easy access, separate from others
                vsc->entities.erase(it);    // erase the entity and update the iterator

                // Handle the skybox-specific components
                auto& textureComp = entity.GetComponent<TextureComponent>();
                vsr->skybox.environmentCube = vsc->projectAssetPool->GetAsset<TextureCubeMap>(textureComp.handle);

                if (!vsr->skybox.environmentCube)
                {
                    vsr->skybox.environmentCube = TextureHandler::GetEmptyTextureCubeMap();
                    vss->displayBackground = false;
                }

                vsr->skybox.irradianceCube->GenerateCubeMap();
                vsr->skybox.prefilteredCube->GenerateCubeMap();

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
        vkGetPhysicalDeviceProperties(vbr->physicalDevice, &deviceProperties);

        // determine required alignment based on min device offset alignment
        size_t minUniformAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        vss->dynamicUniformAlignment = sizeof(glm::mat4);

        if (minUniformAlignment > 0)
            vss->dynamicUniformAlignment = (vss->dynamicUniformAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

        size_t dynamicBufferSize = vsc->entities.size() * vss->dynamicUniformAlignment;

        vsr->uniformDynamicData.model = (glm::mat4*)VulkanMemory::AlignedAlloc(dynamicBufferSize, vss->dynamicUniformAlignment);
        DK_CORE_ASSERT(vsr->uniformDynamicData.model);

        VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        vsr->uniforms.resize(vbr->swapchain.imageCount);
        for (auto& uniform : vsr->uniforms)
        {
            // dynamic uniform buffer object with model matrix
            uniform.dynamic.buffer = VulkanBuffer::Create(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniform.dynamic.descriptor = { uniform.dynamic.buffer.buffer, 0, vss->dynamicUniformAlignment };
            VkCR(vkMapMemory(vbr->device, uniform.dynamic.buffer.memory, 0, dynamicBufferSize, 0, &uniform.dynamic.buffer.mapped));

            // shared uniform buffer object with projection and view matrix
            uniform.shared.buffer = VulkanBuffer::Create(sizeof(vsr->uniformSharedData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.shared.descriptor = { uniform.shared.buffer.buffer, 0, sizeof(vsr->uniformSharedData) };
            VkCR(vkMapMemory(vbr->device, uniform.shared.buffer.memory, 0, sizeof(vsr->uniformSharedData), 0, &uniform.shared.buffer.mapped));

            // uniform buffer object with light data
            uniform.light.buffer = VulkanBuffer::Create(sizeof(vsr->uniformLightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.light.descriptor = { uniform.light.buffer.buffer, 0, sizeof(vsr->uniformLightData) };
            VkCR(vkMapMemory(vbr->device, uniform.light.buffer.memory, 0, sizeof(vsr->uniformLightData), 0, &uniform.light.buffer.mapped));
        }
    }

    void VulkanScene::UpdateUniforms(Ref<EditorCamera> camera)
    {
        // shared scene
        vsr->uniformSharedData.view = camera->GetView();
        vsr->uniformSharedData.projection = camera->GetProjection();

        glm::mat4 cv = glm::inverse(vsr->uniformSharedData.view);
        vsr->uniformSharedData.camPos = glm::vec3(cv[3]);

        // models
        uint32_t index = 0;
        for (Entity entity : vsc->entities)
        {
            // aligned offset for dynamic uniform
            glm::mat4* modelMatrix = (glm::mat4*)(((uint64_t)vsr->uniformDynamicData.model + (index * vss->dynamicUniformAlignment)));

            *modelMatrix = entity.GetComponent<TransformComponent>().GetTransform();

            // auto& modelComp = entity.GetComponent<ModelComponent>();
            // glm::mat4 aabb = s_ProjectAssetPool->GetAsset<Model>(modelComp.handle)->aaBoundingBox;

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

        vsr->uniformLightData.lightDir = glm::vec4(
            sin(vsr->lightSource.rotation.x) * cos(vsr->lightSource.rotation.y),
            sin(vsr->lightSource.rotation.y),
            cos(vsr->lightSource.rotation.x) * cos(vsr->lightSource.rotation.y),
            0.0f);

        UniformSet uniformSet = vsr->uniforms[vbc->currentFrame];
        memcpy(uniformSet.dynamic.buffer.mapped, vsr->uniformDynamicData.model, vss->dynamicUniformAlignment * vsc->entities.size());
        memcpy(uniformSet.shared.buffer.mapped, &vsr->uniformSharedData, sizeof(vsr->uniformSharedData));
        memcpy(uniformSet.light.buffer.mapped, &vsr->uniformLightData, sizeof(vsr->uniformLightData));

        // flush dynamic uniform to make changes visible to the host
        VkMappedMemoryRange memoryRange{};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = uniformSet.dynamic.buffer.memory;
        memoryRange.size = vss->dynamicUniformAlignment * vsc->entities.size();
        vkFlushMappedMemoryRanges(vbr->device, 1, &memoryRange);
    }

    void VulkanScene::SetUpDescriptors()
    {
        /* DESCRIPTOR POOLS */

        {   // per-frame descriptor pool allocators
            std::vector<VulkanDescriptor::PoolSizeRatio> poolSizes = {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }, // 1 set for scene
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (2 + 2) }, // 2 sets for scene, 2 sets for skybox 
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (3 + 1) }, // 3 sets for scene, 1 set for skybox
            };

            for (int i = 0; i < vbs->frameOverlap; i++)
            {
                static uint32_t maxSets = 1000;
                VulkanDescriptor::AllocatorGrowable descriptorAllocator{ maxSets, poolSizes };
                vbc->frames[i].descriptorAllocator = descriptorAllocator;
            }
        }

        {   // static descriptor pool allocator
            uint32_t meshCount = 0;
            uint32_t materialCount = 0;
            uint32_t materialSamplerCount = 0;

            for (Entity entity : vsc->entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();

                Ref<Model> model = vsc->projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);
                for (auto node : model->linearNodes)
                    if (node->mesh) meshCount++; // 1 set for mesh

                uint32_t entityMaterialCount = prefabComp.materialHandles.size();
                materialCount += entityMaterialCount;
                materialSamplerCount += (entityMaterialCount * 5); // 5 sets for material samplers
            }

            std::vector<VulkanDescriptor::PoolSizeRatio> poolSizes = {
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (float)materialSamplerCount },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,(float)meshCount },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }, // 1 set for material buffer
            };

            uint32_t maxSets = meshCount + materialCount + materialSamplerCount + 1;
            VulkanDescriptor::AllocatorGrowable descriptorAllocator{ maxSets, poolSizes };
            vsr->staticDescriptorAllocator = descriptorAllocator;
        }

        /* DESCRIPTOR SETS */

        {   // scene (matrices and environment maps)
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
            layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

            vsr->descriptorLayouts.scene = layoutBuilder.Build();

            for (int i = 0; i < vbs->frameOverlap; i++)
            {
                FrameData& frame = vbc->frames[i];

                VkDescriptorSetLayout layout = vsr->descriptorLayouts.scene;
                frame.sceneDescriptorSet = frame.descriptorAllocator.Allocate(layout);
                VkDescriptorSet set = frame.sceneDescriptorSet;

                UniformSet& uniform = vsr->uniforms[i];

                VulkanDescriptor::Writer writer;
                writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, uniform.dynamic.descriptor);
                writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.shared.descriptor);
                writer.WriteBuffer(2, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.light.descriptor);

                writer.WriteImage(3, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vsr->skybox.irradianceCube->descriptor);
                writer.WriteImage(4, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vsr->skybox.prefilteredCube->descriptor);
                writer.WriteImage(5, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vsr->lightSource.lutBrdf->descriptor);

                writer.UpdateSets();
            }
        }

        {   // skybox
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

            vsr->descriptorLayouts.skybox = layoutBuilder.Build();

            for (int i = 0; i < vbs->frameOverlap; i++)
            {
                FrameData& frame = vbc->frames[i];

                VkDescriptorSetLayout layout = vsr->descriptorLayouts.skybox;
                frame.skyboxDescriptorSet = frame.descriptorAllocator.Allocate(layout);
                VkDescriptorSet set = frame.skyboxDescriptorSet;

                UniformSet& uniform = vsr->uniforms[i];

                VulkanDescriptor::Writer writer;
                writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.shared.descriptor);
                writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.light.descriptor);
                writer.WriteImage(2, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vsr->skybox.prefilteredCube->descriptor);

                writer.UpdateSets();
            }
        }

        {   // material samplers
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

            vsr->descriptorLayouts.material = layoutBuilder.Build();

            Ref<Texture2D> emptyTexture = TextureHandler::GetEmptyTexture2D();

            for (Entity entity : vsc->entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> model = vsc->projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

                for (auto& material : model->materials)
                {
                    std::vector<VkDescriptorImageInfo> materialDescriptors = {
                       emptyTexture->descriptor,
                       emptyTexture->descriptor,
                       material->normalTexture ? material->normalTexture->descriptor : emptyTexture->descriptor,
                       material->occlusionTexture ? material->occlusionTexture->descriptor : emptyTexture->descriptor,
                       material->emissiveTexture ? material->emissiveTexture->descriptor : emptyTexture->descriptor
                    };

                    if (material->pbrWorkflows.metallicRoughness)
                    {
                        if (material->baseColorTexture)
                            materialDescriptors[0] = material->baseColorTexture->descriptor;
                        if (material->metallicRoughnessTexture)
                            materialDescriptors[1] = material->metallicRoughnessTexture->descriptor;
                    }
                    else if (material->pbrWorkflows.specularGlossiness)
                    {
                        if (material->extension.diffuseTexture)
                            materialDescriptors[0] = material->extension.diffuseTexture->descriptor;
                        if (material->extension.specularGlossinessTexture)
                            materialDescriptors[1] = material->extension.specularGlossinessTexture->descriptor;
                    }

                    VkDescriptorSetLayout layout = vsr->descriptorLayouts.material;
                    material->descriptorSet = vsr->staticDescriptorAllocator.Allocate(layout);
                    VkDescriptorSet set = material->descriptorSet;

                    VulkanDescriptor::Writer writer;
                    writer.WriteImage(0, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, materialDescriptors[0]);
                    writer.WriteImage(1, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, materialDescriptors[1]);
                    writer.WriteImage(2, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, materialDescriptors[2]);
                    writer.WriteImage(3, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, materialDescriptors[3]);
                    writer.WriteImage(4, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, materialDescriptors[4]);

                    writer.UpdateSets();
                }
            }
        }

        {   // model node matrices
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);

            vsr->descriptorLayouts.node = layoutBuilder.Build();

            auto AllocateNodeDescriptorSet = [](Node* node)
                {
                    if (!node || !node->mesh) return;

                    VkDescriptorSetLayout layout = vsr->descriptorLayouts.node;
                    node->mesh->uniform.descriptorSet = vsr->staticDescriptorAllocator.Allocate(layout);
                    VkDescriptorSet set = node->mesh->uniform.descriptorSet;

                    VulkanDescriptor::Writer writer;
                    writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, node->mesh->uniform.descriptor);

                    writer.UpdateSets();
                };

            for (Entity entity : vsc->entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> model = vsc->projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

                for (auto& node : model->nodes) // per-node descriptor set
                {
                    AllocateNodeDescriptorSet(node);

                    for (auto child : node->children)
                        AllocateNodeDescriptorSet(child); // recursive
                }
            }
        }

        {   // material buffer
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

            vsr->descriptorLayouts.materialBuffer = layoutBuilder.Build();

            VkDescriptorSetLayout layout = vsr->descriptorLayouts.materialBuffer;
            vsr->materialBuffer.descriptorSet = vsr->staticDescriptorAllocator.Allocate(layout);
            VkDescriptorSet set = vsr->materialBuffer.descriptorSet;

            VulkanDescriptor::Writer writer;
            writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, vsr->materialBuffer.descriptor);

            writer.UpdateSets();
        }
    }

    void VulkanScene::SetUpPipelines()
    {
        VkShaderModule skyboxVert = VulkanShader::CreateModule("skybox.vert.spv");
        VkShaderModule skyboxFrag = VulkanShader::CreateModule("skybox.frag.spv");

        VkShaderModule pbrVert = VulkanShader::CreateModule("pbr.vert.spv");
        VkShaderModule pbrMaterialFrag = VulkanShader::CreateModule("material_pbr.frag.spv");
        VkShaderModule unlitMaterialFrag = VulkanShader::CreateModule("material_unlit.frag.spv");

        /* PIPELINE LAYOUTS */

        std::vector<VkDescriptorSetLayout> skyboxDescriptorLayouts = {
              vsr->descriptorLayouts.skybox,
        };

        std::vector<VkDescriptorSetLayout> sceneDescriptorLayouts = {
               vsr->descriptorLayouts.scene,
               vsr->descriptorLayouts.material,
               vsr->descriptorLayouts.node,
               vsr->descriptorLayouts.materialBuffer,
        };

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.size = sizeof(uint32_t);
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(skyboxDescriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = skyboxDescriptorLayouts.data();
        VkCR(vkCreatePipelineLayout(vbr->device, &pipelineLayoutInfo, nullptr, &vsr->skyboxPipelineLayout));

        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(sceneDescriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = sceneDescriptorLayouts.data();
        VkCR(vkCreatePipelineLayout(vbr->device, &pipelineLayoutInfo, nullptr, &vsr->scenePipelineLayout));

        /* PIPELINE */

        VkSampleCountFlagBits sampleFlag = vbs->multiSampling ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT;

        VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

        std::vector<VkVertexInputAttributeDescription>  vertexInputAttributes;

        {   // skybox pipeline
            VulkanPipeline::Builder pipelineBuilder;
            pipelineBuilder.SetShaders(skyboxVert, skyboxFrag);
            pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
            pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            pipelineBuilder.DisableColorBlending();
            pipelineBuilder.SetMultisampling(sampleFlag);
            pipelineBuilder.DisableDepthTest();

            vertexInputAttributes = {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos) },
                { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal) },
                { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv0) },
            };

            pipelineBuilder.SetVertexInput(vertexInputBinding, vertexInputAttributes);

            vsr->pipelines.skybox = pipelineBuilder.Build(vsr->skyboxPipelineLayout);
        }

        {   // pbr default pipeline
            VulkanPipeline::Builder pipelineBuilder;
            pipelineBuilder.SetShaders(pbrVert, pbrMaterialFrag);
            pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
            pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            pipelineBuilder.DisableColorBlending();
            pipelineBuilder.SetMultisampling(sampleFlag);
            pipelineBuilder.EnableDepthTest();

            vertexInputAttributes = {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos)},
                { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal) },
                { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv0) },
                { 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv1) },
                { 4, 0, VK_FORMAT_R32G32B32A32_UINT, offsetof(Model::Vertex, joint0) },
                { 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, weight0) },
                { 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, color) }
            };

            pipelineBuilder.SetVertexInput(vertexInputBinding, vertexInputAttributes);

            vsr->pipelines.pbr = pipelineBuilder.Build(vsr->scenePipelineLayout);

            // pbr double sided pipeline
            pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            vsr->pipelines.pbrDoubleSided = pipelineBuilder.Build(vsr->scenePipelineLayout);

            // pbr alpha blending pipeline
            pipelineBuilder.EnableAlphaBlending();
            vsr->pipelines.pbrAlphaBlending = pipelineBuilder.Build(vsr->scenePipelineLayout);
        }

        {   // unlit default pipeline
            VulkanPipeline::Builder pipelineBuilder;
            pipelineBuilder.SetShaders(pbrVert, unlitMaterialFrag);
            pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
            pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            pipelineBuilder.DisableColorBlending();
            pipelineBuilder.SetMultisampling(sampleFlag);
            pipelineBuilder.EnableDepthTest();

            vertexInputAttributes = {
               { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos)},
               { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal) },
               { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv0) },
               { 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv1) },
               { 4, 0, VK_FORMAT_R32G32B32A32_UINT, offsetof(Model::Vertex, joint0) },
               { 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, weight0) },
               { 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, color) }
            };

            pipelineBuilder.SetVertexInput(vertexInputBinding, vertexInputAttributes);

            vsr->pipelines.unlit = pipelineBuilder.Build(vsr->scenePipelineLayout);

            // unlit double sided pipeline
            pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            vsr->pipelines.unlitDoubleSided = pipelineBuilder.Build(vsr->scenePipelineLayout);

            // unlit alpha blending pipeline
            pipelineBuilder.EnableAlphaBlending();
            vsr->pipelines.unlitAlphaBlending = pipelineBuilder.Build(vsr->scenePipelineLayout);
        }

        vkDestroyShaderModule(vbr->device, skyboxVert, nullptr);
        vkDestroyShaderModule(vbr->device, skyboxFrag, nullptr);
        vkDestroyShaderModule(vbr->device, pbrVert, nullptr);
        vkDestroyShaderModule(vbr->device, pbrMaterialFrag, nullptr);
        vkDestroyShaderModule(vbr->device, unlitMaterialFrag, nullptr);
    }

    void VulkanScene::Draw(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VulkanImage::AllocatedImage& colorTarget = vbr->multisampleTarget.color;
        VulkanImage::AllocatedImage& depthTarget = vbr->multisampleTarget.depth;
        VulkanImage::AllocatedImage& viewportImage = vbr->viewport.images[imageIndex];

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        colorAttachment.resolveImageView = viewportImage.view;
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { {0.0065f, 0.005f, 0.0055f, 1.0f} };

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

        if (vss->displayBackground)
        {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vsr->skyboxPipelineLayout, 0, 1, &vbc->frames[vbc->currentFrame].skyboxDescriptorSet, 0, nullptr);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vsr->pipelines.skybox);

            vsr->skybox.model->Draw(commandBuffer);
        }

        VkDeviceSize vertexOffsets[1] = { 0 };
        uint32_t dynamicOffset = 0;

        uint32_t index = 0;
        for (Entity entity : vsc->entities)
        {
            auto& prefabComp = entity.GetComponent<PrefabComponent>();
            Ref<Model> model = vsc->projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertices.buffer, vertexOffsets);

            if (model->indices.buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(commandBuffer, model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            vsc->boundPipeline = VK_NULL_HANDLE;

            // one dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
            dynamicOffset = index * static_cast<uint32_t>(vss->dynamicUniformAlignment);

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

}
