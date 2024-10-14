#include "VulkanScene.h"
#include "dkpch.h"

#include "Deako/Asset/Scene/SceneHandler.h"
#include "Deako/Asset/Texture/TextureHandler.h"

#include "VulkanPipeline.h"

namespace Deako {

    static Ref<VulkanResources> vr = VulkanBase::GetResources();
    static Ref<VulkanContext> vc = VulkanBase::GetContext();
    static Ref<VulkanSettings> vs = VulkanBase::GetSettings();

    void VulkanScene::Build()
    {
        VulkanBase::Idle();

        SetUpAssets();

        SetUpUniforms();

        SetUpDescriptors();

        SetUpPipelines();

        s_SceneValid = true;
    }

    void VulkanScene::CleanUp()
    {
        VulkanBase::Idle();

        vkDestroyPipeline(vr->device, vr->skyboxPipeline, nullptr);
        vkDestroyPipeline(vr->device, vr->pbrPipeline, nullptr);
        vkDestroyPipeline(vr->device, vr->pbrDoubleSidedPipeline, nullptr);
        vkDestroyPipeline(vr->device, vr->pbrAlphaBlendingPipeline, nullptr);
        vkDestroyPipeline(vr->device, vr->unlitPipeline, nullptr);
        vkDestroyPipeline(vr->device, vr->unlitDoubleSidedPipeline, nullptr);
        vkDestroyPipeline(vr->device, vr->unlitAlphaBlendingPipeline, nullptr);

        vkDestroyPipelineLayout(vr->device, vr->scenePipelineLayout, nullptr);
        vkDestroyPipelineLayout(vr->device, vr->skyboxPipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorLayouts.scene, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorLayouts.skybox, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorLayouts.material, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorLayouts.node, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorLayouts.materialBuffer, nullptr);

        for (auto& frame : vc->frames)
            frame.descriptorAllocator.DestroyPools();

        vr->staticDescriptorAllocator.DestroyPools();

        for (auto& uniform : vr->uniforms)
        {
            VulkanBuffer::Destroy(uniform.dynamic.buffer);
            VulkanBuffer::Destroy(uniform.shared.buffer);
            VulkanBuffer::Destroy(uniform.light.buffer);
        }

        VulkanBuffer::Destroy(vr->materialBuffer.buffer);

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
        s_ProjectAssetPool = ProjectAssetPool::Get();

        Ref<Scene> scene = SceneHandler::GetActiveScene();
        vr->entities = scene->GetAllEntitiesWith<PrefabComponent>();

        for (auto it = vr->entities.begin(); it != vr->entities.end(); )
        {
            Entity entity = *it;
            std::string& tag = entity.GetComponent<TagComponent>().tag;

            if (tag == "Skybox")
            {
                vs->displayBackground = true;

                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> mesh = s_ProjectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

                vr->skybox.model = mesh;  // easy access, separate from others
                vr->entities.erase(it);    // erase the entity and update the iterator

                // Handle the skybox-specific components
                auto& textureComp = entity.GetComponent<TextureComponent>();
                vr->skybox.environmentCube = s_ProjectAssetPool->GetAsset<TextureCubeMap>(textureComp.handle);

                if (!vr->skybox.environmentCube)
                {
                    vr->skybox.environmentCube = TextureHandler::GetEmptyTextureCubeMap();
                    vs->displayBackground = false;
                }

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

        vr->uniformDynamicData.model = (glm::mat4*)VulkanMemory::AlignedAlloc(dynamicBufferSize, vr->dynamicUniformAlignment);
        DK_CORE_ASSERT(vr->uniformDynamicData.model);

        VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        vr->uniforms.resize(vr->swapchain.imageCount);
        for (auto& uniform : vr->uniforms)
        {
            // dynamic uniform buffer object with model matrix
            uniform.dynamic.buffer = VulkanBuffer::Create(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniform.dynamic.descriptor = { uniform.dynamic.buffer.buffer, 0, vr->dynamicUniformAlignment };
            VkCR(vkMapMemory(vr->device, uniform.dynamic.buffer.memory, 0, dynamicBufferSize, 0, &uniform.dynamic.buffer.mapped));

            // shared uniform buffer object with projection and view matrix
            uniform.shared.buffer = VulkanBuffer::Create(sizeof(vr->uniformSharedData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.shared.descriptor = { uniform.shared.buffer.buffer, 0, sizeof(vr->uniformSharedData) };
            VkCR(vkMapMemory(vr->device, uniform.shared.buffer.memory, 0, sizeof(vr->uniformSharedData), 0, &uniform.shared.buffer.mapped));

            // uniform buffer object with light data
            uniform.light.buffer = VulkanBuffer::Create(sizeof(vr->uniformLightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.light.descriptor = { uniform.light.buffer.buffer, 0, sizeof(vr->uniformLightData) };
            VkCR(vkMapMemory(vr->device, uniform.light.buffer.memory, 0, sizeof(vr->uniformLightData), 0, &uniform.light.buffer.mapped));
        }
    }

    void VulkanScene::UpdateUniforms(Ref<EditorCamera> camera)
    {
        // shared scene
        vr->uniformSharedData.view = camera->GetView();
        vr->uniformSharedData.projection = camera->GetProjection();

        glm::mat4 cv = glm::inverse(vr->uniformSharedData.view);
        vr->uniformSharedData.camPos = glm::vec3(cv[3]);

        // models
        uint32_t index = 0;
        for (Entity entity : vr->entities)
        {
            // aligned offset for dynamic uniform
            glm::mat4* modelMatrix = (glm::mat4*)(((uint64_t)vr->uniformDynamicData.model + (index * vr->dynamicUniformAlignment)));

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

        vr->uniformLightData.lightDir = glm::vec4(
            sin(vr->lightSource.rotation.x) * cos(vr->lightSource.rotation.y),
            sin(vr->lightSource.rotation.y),
            cos(vr->lightSource.rotation.x) * cos(vr->lightSource.rotation.y),
            0.0f);

        VulkanResources::UniformSet uniformSet = vr->uniforms[vc->currentFrame];
        memcpy(uniformSet.dynamic.buffer.mapped, vr->uniformDynamicData.model, vr->dynamicUniformAlignment * vr->entities.size());
        memcpy(uniformSet.shared.buffer.mapped, &vr->uniformSharedData, sizeof(vr->uniformSharedData));
        memcpy(uniformSet.light.buffer.mapped, &vr->uniformLightData, sizeof(vr->uniformLightData));

        // flush dynamic uniform to make changes visible to the host
        VkMappedMemoryRange memoryRange{};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = uniformSet.dynamic.buffer.memory;
        memoryRange.size = vr->dynamicUniformAlignment * vr->entities.size();
        vkFlushMappedMemoryRanges(vr->device, 1, &memoryRange);
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

            for (int i = 0; i < vs->frameOverlap; i++)
            {
                static uint32_t maxSets = 1000;
                VulkanDescriptor::AllocatorGrowable descriptorAllocator{ maxSets, poolSizes };
                vc->frames[i].descriptorAllocator = descriptorAllocator;
            }
        }

        {   // static descriptor pool allocator
            uint32_t meshCount = 0;
            uint32_t materialCount = 0;
            uint32_t materialSamplerCount = 0;

            for (Entity entity : vr->entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();

                Ref<Model> model = s_ProjectAssetPool->GetAsset<Model>(prefabComp.meshHandle);
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
            vr->staticDescriptorAllocator = descriptorAllocator;
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

            vr->descriptorLayouts.scene = layoutBuilder.Build();

            for (int i = 0; i < vs->frameOverlap; i++)
            {
                FrameData& frame = vc->frames[i];

                VkDescriptorSetLayout layout = vr->descriptorLayouts.scene;
                frame.sceneDescriptorSet = frame.descriptorAllocator.Allocate(layout);
                VkDescriptorSet set = frame.sceneDescriptorSet;

                VulkanResources::UniformSet& uniform = vr->uniforms[i];

                VulkanDescriptor::Writer writer;
                writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, uniform.dynamic.descriptor);
                writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.shared.descriptor);
                writer.WriteBuffer(2, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.light.descriptor);

                writer.WriteImage(3, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vr->skybox.irradianceCube->descriptor);
                writer.WriteImage(4, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vr->skybox.prefilteredCube->descriptor);
                writer.WriteImage(5, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vr->textures.lutBrdf->descriptor);

                writer.UpdateSets();
            }
        }

        {   // skybox
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

            vr->descriptorLayouts.skybox = layoutBuilder.Build();

            for (int i = 0; i < vs->frameOverlap; i++)
            {
                FrameData& frame = vc->frames[i];

                VkDescriptorSetLayout layout = vr->descriptorLayouts.skybox;
                frame.skyboxDescriptorSet = frame.descriptorAllocator.Allocate(layout);
                VkDescriptorSet set = frame.skyboxDescriptorSet;

                VulkanResources::UniformSet& uniform = vr->uniforms[i];

                VulkanDescriptor::Writer writer;
                writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.shared.descriptor);
                writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.light.descriptor);
                writer.WriteImage(2, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vr->skybox.prefilteredCube->descriptor);

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

            vr->descriptorLayouts.material = layoutBuilder.Build();

            Ref<Texture2D> emptyTexture = TextureHandler::GetEmptyTexture2D();

            for (Entity entity : vr->entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> model = s_ProjectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

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

                    VkDescriptorSetLayout layout = vr->descriptorLayouts.material;
                    material->descriptorSet = vr->staticDescriptorAllocator.Allocate(layout);
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

            vr->descriptorLayouts.node = layoutBuilder.Build();

            auto AllocateNodeDescriptorSet = [](Node* node)
                {
                    if (!node || !node->mesh) return;

                    VkDescriptorSetLayout layout = vr->descriptorLayouts.node;
                    node->mesh->uniform.descriptorSet = vr->staticDescriptorAllocator.Allocate(layout);
                    VkDescriptorSet set = node->mesh->uniform.descriptorSet;

                    VulkanDescriptor::Writer writer;
                    writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, node->mesh->uniform.descriptor);

                    writer.UpdateSets();
                };

            for (Entity entity : vr->entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> model = s_ProjectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

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

            vr->descriptorLayouts.materialBuffer = layoutBuilder.Build();

            VkDescriptorSetLayout layout = vr->descriptorLayouts.materialBuffer;
            vr->materialBuffer.descriptorSet = vr->staticDescriptorAllocator.Allocate(layout);
            VkDescriptorSet set = vr->materialBuffer.descriptorSet;

            VulkanDescriptor::Writer writer;
            writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, vr->materialBuffer.descriptor);

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
              vr->descriptorLayouts.skybox,
        };

        std::vector<VkDescriptorSetLayout> sceneDescriptorLayouts = {
               vr->descriptorLayouts.scene,
               vr->descriptorLayouts.material,
               vr->descriptorLayouts.node,
               vr->descriptorLayouts.materialBuffer,
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
        VkCR(vkCreatePipelineLayout(vr->device, &pipelineLayoutInfo, nullptr, &vr->skyboxPipelineLayout));

        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(sceneDescriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = sceneDescriptorLayouts.data();
        VkCR(vkCreatePipelineLayout(vr->device, &pipelineLayoutInfo, nullptr, &vr->scenePipelineLayout));

        /* PIPELINE */

        VkSampleCountFlagBits sampleFlag = vs->multiSampling ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT;

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

            vr->skyboxPipeline = pipelineBuilder.Build(vr->skyboxPipelineLayout);
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

            vr->pbrPipeline = pipelineBuilder.Build(vr->scenePipelineLayout);

            // pbr double sided pipeline
            pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            vr->pbrDoubleSidedPipeline = pipelineBuilder.Build(vr->scenePipelineLayout);

            // pbr alpha blending pipeline
            pipelineBuilder.EnableAlphaBlending();
            vr->pbrAlphaBlendingPipeline = pipelineBuilder.Build(vr->scenePipelineLayout);
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

            vr->unlitPipeline = pipelineBuilder.Build(vr->scenePipelineLayout);

            // unlit double sided pipeline
            pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            vr->unlitDoubleSidedPipeline = pipelineBuilder.Build(vr->scenePipelineLayout);

            // unlit alpha blending pipeline
            pipelineBuilder.EnableAlphaBlending();
            vr->unlitAlphaBlendingPipeline = pipelineBuilder.Build(vr->scenePipelineLayout);
        }

        vkDestroyShaderModule(vr->device, skyboxVert, nullptr);
        vkDestroyShaderModule(vr->device, skyboxFrag, nullptr);
        vkDestroyShaderModule(vr->device, pbrVert, nullptr);
        vkDestroyShaderModule(vr->device, pbrMaterialFrag, nullptr);
        vkDestroyShaderModule(vr->device, unlitMaterialFrag, nullptr);
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

        if (vs->displayBackground)
        {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->skyboxPipelineLayout, 0, 1, &vc->frames[vc->currentFrame].skyboxDescriptorSet, 0, nullptr);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->skyboxPipeline);

            vr->skybox.model->Draw(commandBuffer);
        }

        VkDeviceSize vertexOffsets[1] = { 0 };
        uint32_t dynamicOffset = 0;

        uint32_t index = 0;
        for (Entity entity : vr->entities)
        {
            auto& prefabComp = entity.GetComponent<PrefabComponent>();
            Ref<Model> model = s_ProjectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertices.buffer, vertexOffsets);

            if (model->indices.buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(commandBuffer, model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            vc->boundPipeline = VK_NULL_HANDLE;

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

}
