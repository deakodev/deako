#include "VulkanScene.h"
#include "dkpch.h"

#include "Deako/Asset/Scene/SceneHandler.h"
#include "Deako/Asset/Texture/TextureHandler.h"
#include "Deako/Core/Input.h" 
#include "Deako/ImGui/ImGuiLayer.h" 

#include "VulkanBase.h"
#include "VulkanPipeline.h"

namespace Deako {

    static Ref<VulkanBaseResources> vb = VulkanBase::GetResources();

    void VulkanScene::Build()
    {
        VulkanBase::Idle();

        SetUpAssets();

        SetUpUniforms();

        SetUpDescriptors();

        VkExtent3D targetExtent = { vb->swapchain.extent.width, vb->swapchain.extent.height, 1 };
        VkSampleCountFlagBits targetSamples = VK_SAMPLE_COUNT_1_BIT;

        VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkImageUsageFlags colorUsages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vs->picker.colorTarget =
            VulkanImage::Create(targetExtent, colorFormat, targetSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        VkImageUsageFlags depthUsages = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vs->picker.depthTarget =
            VulkanImage::Create(targetExtent, depthFormat, targetSamples, depthUsages, 1, VK_IMAGE_TYPE_2D);

        vs->picker.stagingBuffer = VulkanBuffer::Create(sizeof(glm::vec4), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vb->device, vs->picker.stagingBuffer.memory, 0, sizeof(glm::vec4), 0, &vs->picker.stagingBuffer.mapped));

        SetUpPipelines();

        vs->context.scene->activeCamera->ResizeCamera({ vb->swapchain.extent.width, vb->swapchain.extent.height });

        vs->context.scene->isValid = true;
    }

    void VulkanScene::CleanUp()
    {
        VulkanBase::Idle();

        vkDestroyPipeline(vb->device, vs->pipelines.skybox, nullptr);
        vkDestroyPipeline(vb->device, vs->pipelines.pbr, nullptr);
        vkDestroyPipeline(vb->device, vs->pipelines.pbrDoubleSided, nullptr);
        vkDestroyPipeline(vb->device, vs->pipelines.pbrAlphaBlending, nullptr);
        vkDestroyPipeline(vb->device, vs->pipelines.unlit, nullptr);
        vkDestroyPipeline(vb->device, vs->pipelines.unlitDoubleSided, nullptr);
        vkDestroyPipeline(vb->device, vs->pipelines.unlitAlphaBlending, nullptr);

        vkDestroyPipelineLayout(vb->device, vs->scenePipelineLayout, nullptr);
        vkDestroyPipelineLayout(vb->device, vs->skyboxPipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(vb->device, vs->descriptorLayouts.scene, nullptr);
        vkDestroyDescriptorSetLayout(vb->device, vs->descriptorLayouts.skybox, nullptr);
        vkDestroyDescriptorSetLayout(vb->device, vs->descriptorLayouts.material, nullptr);
        vkDestroyDescriptorSetLayout(vb->device, vs->descriptorLayouts.node, nullptr);
        vkDestroyDescriptorSetLayout(vb->device, vs->descriptorLayouts.materialBuffer, nullptr);

        for (auto& frame : vb->frames)
            frame.descriptorAllocator.DestroyPools();

        vs->staticDescriptorAllocator.DestroyPools();

        for (auto& uniform : vs->uniforms)
        {
            VulkanBuffer::Destroy(uniform.dynamic.buffer);
            VulkanBuffer::Destroy(uniform.shared.buffer);
            VulkanBuffer::Destroy(uniform.light.buffer);
        }

        VulkanBuffer::Destroy(vs->materialBuffer.buffer);

        vs->lightSource.lutBrdf->Destroy();
        vs->skybox.irradianceCube->Destroy();
        vs->skybox.prefilteredCube->Destroy();

        vs->context.entities.clear();
    }

    void VulkanScene::Rebuild()
    {
        CleanUp();
        Build();
    }

    void VulkanScene::SetUpAssets()
    {
        vs->context.projectAssetPool = ProjectAssetPool::Get();

        vs->context.scene = SceneHandler::GetActiveScene();

        vs->context.entities = vs->context.scene->GetAllEntitiesWith<PrefabComponent>();

        for (auto it = vs->context.entities.begin(); it != vs->context.entities.end(); )
        {
            Entity entity = *it;
            std::string& tag = entity.GetComponent<TagComponent>().tag;

            if (tag == "Skybox")
            {
                vs->settings.displayBackground = true;

                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> mesh = vs->context.projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

                vs->skybox.model = mesh; // easy access, separate from others
                vs->context.entities.erase(it); // erase the entity and update the iterator

                // Handle the skybox-specific components
                auto& textureComp = entity.GetComponent<TextureComponent>();
                vs->skybox.environmentCube = vs->context.projectAssetPool->GetAsset<TextureCubeMap>(textureComp.handle);

                if (!vs->skybox.environmentCube)
                {
                    vs->skybox.environmentCube = TextureHandler::GetEmptyTextureCubeMap();
                    vs->settings.displayBackground = false;
                }

                vs->skybox.irradianceCube->GenerateCubeMap();
                vs->skybox.prefilteredCube->GenerateCubeMap();

                break;
            }

            ++it;
        }

        CreateMaterialBuffer();
        GenerateBRDFLookUpTable();
    }

    void VulkanScene::SetUpUniforms()
    {
        // dynamic uniform alignments
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vb->physicalDevice, &deviceProperties);

        // determine required alignment based on min device offset alignment
        size_t minUniformAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        vs->dynamicUniformAlignment = sizeof(glm::mat4);
        vs->pickerUniformAlignment = sizeof(glm::vec4);

        if (minUniformAlignment > 0)
        {
            vs->dynamicUniformAlignment = (vs->dynamicUniformAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);
            vs->pickerUniformAlignment = (vs->pickerUniformAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);
        }

        size_t dynamicBufferSize = vs->context.entities.size() * vs->dynamicUniformAlignment;
        size_t pickerBufferSize = vs->context.entities.size() * vs->pickerUniformAlignment;

        vs->uniformDynamicData.model = (glm::mat4*)VulkanMemory::AlignedAlloc(dynamicBufferSize, vs->dynamicUniformAlignment);
        vs->uniformPickerData.colorID = (glm::vec4*)VulkanMemory::AlignedAlloc(pickerBufferSize, vs->pickerUniformAlignment);

        DK_CORE_ASSERT(vs->uniformDynamicData.model);
        DK_CORE_ASSERT(vs->uniformPickerData.colorID);

        VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        vs->uniforms.resize(vb->swapchain.imageCount);
        for (auto& uniform : vs->uniforms)
        {
            // dynamic uniform buffer object with model matrix
            uniform.dynamic.buffer = VulkanBuffer::Create(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniform.dynamic.descriptor = { uniform.dynamic.buffer.buffer, 0, vs->dynamicUniformAlignment };
            VkCR(vkMapMemory(vb->device, uniform.dynamic.buffer.memory, 0, dynamicBufferSize, 0, &uniform.dynamic.buffer.mapped));

            uniform.picker.buffer = VulkanBuffer::Create(pickerBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniform.picker.descriptor = { uniform.picker.buffer.buffer, 0, vs->pickerUniformAlignment };
            VkCR(vkMapMemory(vb->device, uniform.picker.buffer.memory, 0, pickerBufferSize, 0, &uniform.picker.buffer.mapped));

            // shared uniform buffer object with projection and view matrix
            uniform.shared.buffer = VulkanBuffer::Create(sizeof(vs->uniformSharedData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.shared.descriptor = { uniform.shared.buffer.buffer, 0, sizeof(vs->uniformSharedData) };
            VkCR(vkMapMemory(vb->device, uniform.shared.buffer.memory, 0, sizeof(vs->uniformSharedData), 0, &uniform.shared.buffer.mapped));

            // light uniform buffer object
            uniform.light.buffer = VulkanBuffer::Create(sizeof(vs->uniformLightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.light.descriptor = { uniform.light.buffer.buffer, 0, sizeof(vs->uniformLightData) };
            VkCR(vkMapMemory(vb->device, uniform.light.buffer.memory, 0, sizeof(vs->uniformLightData), 0, &uniform.light.buffer.mapped));
        }
    }

    void VulkanScene::UpdateUniforms()
    {
        Ref<EditorCamera> camera = vs->context.scene->activeCamera;
        // shared scene
        vs->uniformSharedData.view = camera->GetView();
        vs->uniformSharedData.projection = camera->GetProjection();

        glm::mat4 cv = glm::inverse(vs->uniformSharedData.view);
        vs->uniformSharedData.camPos = glm::vec3(cv[3]);

        // models
        uint32_t index = 0;
        for (Entity entity : vs->context.entities)
        {
            // aligned offset for dynamic uniform (model and colorID separately)
            glm::mat4* modelMatrix = (glm::mat4*)(((uint64_t)vs->uniformDynamicData.model) + (index * vs->dynamicUniformAlignment));
            *modelMatrix = entity.GetComponent<TransformComponent>().GetTransform();

            glm::vec4* colorID = (glm::vec4*)(((uint64_t)vs->uniformPickerData.colorID) + (index * vs->pickerUniformAlignment));

            uint32_t entityID = static_cast<uint32_t>(entity);

            float r = ((entityID >> 16) & 0x0FF) / 255.0f;
            float g = ((entityID >> 8) & 0x0FF) / 255.0f;
            float b = ((entityID) & 0x0FF) / 255.0f;

            *colorID = glm::vec4(r, g, b, 1.0f);

            index++;
        }

        vs->uniformLightData.lightDir = glm::vec4(
            sin(vs->lightSource.rotation.x) * cos(vs->lightSource.rotation.y),
            sin(vs->lightSource.rotation.y),
            cos(vs->lightSource.rotation.x) * cos(vs->lightSource.rotation.y),
            0.0f);

        UniformSet uniformSet = vs->uniforms[vb->context.currentFrame];
        memcpy(uniformSet.dynamic.buffer.mapped, vs->uniformDynamicData.model, vs->dynamicUniformAlignment * vs->context.entities.size());
        memcpy(uniformSet.picker.buffer.mapped, vs->uniformPickerData.colorID, vs->pickerUniformAlignment * vs->context.entities.size());
        memcpy(uniformSet.shared.buffer.mapped, &vs->uniformSharedData, sizeof(vs->uniformSharedData));
        memcpy(uniformSet.light.buffer.mapped, &vs->uniformLightData, sizeof(vs->uniformLightData));

        // flush dynamic uniform to make changes visible to the host
        VkMappedMemoryRange memoryRange{};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = uniformSet.dynamic.buffer.memory;
        memoryRange.size = vs->dynamicUniformAlignment * vs->context.entities.size();
        vkFlushMappedMemoryRanges(vb->device, 1, &memoryRange);

        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = uniformSet.picker.buffer.memory;
        memoryRange.size = vs->pickerUniformAlignment * vs->context.entities.size();
        vkFlushMappedMemoryRanges(vb->device, 1, &memoryRange);
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

            for (int i = 0; i < vb->settings.frameOverlap; i++)
            {
                static uint32_t maxSets = 1000;
                VulkanDescriptor::AllocatorGrowable descriptorAllocator{ maxSets, poolSizes };
                vb->frames[i].descriptorAllocator = descriptorAllocator;
            }
        }

        {   // static descriptor pool allocator
            uint32_t meshCount = 0;
            uint32_t materialCount = 0;
            uint32_t materialSamplerCount = 0;

            for (Entity entity : vs->context.entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();

                Ref<Model> model = vs->context.projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);
                for (auto node : model->linearNodes)
                    if (node->mesh) meshCount++; // 1 set for mesh

                uint32_t entityMaterialCount = prefabComp.materialHandles.size();
                materialCount += entityMaterialCount;
                materialSamplerCount += (entityMaterialCount * 5); // 5 sets for material samplers
            }

            std::vector<VulkanDescriptor::PoolSizeRatio> poolSizes = {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }, // 1 set for picker
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (float)materialSamplerCount },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,(float)meshCount },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }, // 1 set for material buffer
            };

            uint32_t maxSets = meshCount + materialCount + materialSamplerCount + 1;
            VulkanDescriptor::AllocatorGrowable descriptorAllocator{ maxSets, poolSizes };
            vs->staticDescriptorAllocator = descriptorAllocator;
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

            vs->descriptorLayouts.scene = layoutBuilder.Build();

            for (int i = 0; i < vb->settings.frameOverlap; i++)
            {
                FrameData& frame = vb->frames[i];

                VkDescriptorSetLayout layout = vs->descriptorLayouts.scene;
                frame.sceneDescriptorSet = frame.descriptorAllocator.Allocate(layout);
                VkDescriptorSet set = frame.sceneDescriptorSet;

                UniformSet& uniform = vs->uniforms[i];

                VulkanDescriptor::Writer writer;
                writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, uniform.dynamic.descriptor);
                writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.shared.descriptor);
                writer.WriteBuffer(2, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.light.descriptor);

                writer.WriteImage(3, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vs->skybox.irradianceCube->descriptor);
                writer.WriteImage(4, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vs->skybox.prefilteredCube->descriptor);
                writer.WriteImage(5, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vs->lightSource.lutBrdf->descriptor);

                writer.UpdateSets();
            }
        }

        {   // skybox
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

            vs->descriptorLayouts.skybox = layoutBuilder.Build();

            for (int i = 0; i < vb->settings.frameOverlap; i++)
            {
                FrameData& frame = vb->frames[i];

                VkDescriptorSetLayout layout = vs->descriptorLayouts.skybox;
                frame.skyboxDescriptorSet = frame.descriptorAllocator.Allocate(layout);
                VkDescriptorSet set = frame.skyboxDescriptorSet;

                UniformSet& uniform = vs->uniforms[i];

                VulkanDescriptor::Writer writer;
                writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.shared.descriptor);
                writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.light.descriptor);
                writer.WriteImage(2, set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vs->skybox.prefilteredCube->descriptor);

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

            vs->descriptorLayouts.material = layoutBuilder.Build();

            Ref<Texture2D> emptyTexture = TextureHandler::GetEmptyTexture2D();

            for (Entity entity : vs->context.entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> model = vs->context.projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

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

                    VkDescriptorSetLayout layout = vs->descriptorLayouts.material;
                    material->descriptorSet = vs->staticDescriptorAllocator.Allocate(layout);
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

            vs->descriptorLayouts.node = layoutBuilder.Build();

            auto AllocateNodeDescriptorSet = [](Node* node)
                {
                    if (!node || !node->mesh) return;

                    VkDescriptorSetLayout layout = vs->descriptorLayouts.node;
                    node->mesh->uniform.descriptorSet = vs->staticDescriptorAllocator.Allocate(layout);
                    VkDescriptorSet set = node->mesh->uniform.descriptorSet;

                    VulkanDescriptor::Writer writer;
                    writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, node->mesh->uniform.descriptor);

                    writer.UpdateSets();
                };

            for (Entity entity : vs->context.entities)
            {
                auto& prefabComp = entity.GetComponent<PrefabComponent>();
                Ref<Model> model = vs->context.projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

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

            vs->descriptorLayouts.materialBuffer = layoutBuilder.Build();

            VkDescriptorSetLayout layout = vs->descriptorLayouts.materialBuffer;
            vs->materialBuffer.descriptorSet = vs->staticDescriptorAllocator.Allocate(layout);
            VkDescriptorSet set = vs->materialBuffer.descriptorSet;

            VulkanDescriptor::Writer writer;
            writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, vs->materialBuffer.descriptor);

            writer.UpdateSets();
        }

        {   // color picker
            VulkanDescriptor::LayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT);
            layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
            layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);

            vs->descriptorLayouts.picker = layoutBuilder.Build();

            VkDescriptorSetLayout layout = vs->descriptorLayouts.picker;
            vs->picker.descriptorSet = vs->staticDescriptorAllocator.Allocate(layout);
            VkDescriptorSet set = vs->picker.descriptorSet;

            UniformSet& uniform = vs->uniforms[0]; // any of the dynamic descs should work?

            VulkanDescriptor::Writer writer;
            writer.WriteBuffer(0, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, uniform.picker.descriptor);
            writer.WriteBuffer(1, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, uniform.dynamic.descriptor);
            writer.WriteBuffer(2, set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform.shared.descriptor);

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

        VkShaderModule pickerVert = VulkanShader::CreateModule("picker.vert.spv");
        VkShaderModule pickerFrag = VulkanShader::CreateModule("picker.frag.spv");

        /* PIPELINE LAYOUTS */
        std::vector<VkDescriptorSetLayout> skyboxDescriptorLayouts = {
              vs->descriptorLayouts.skybox,
        };

        std::vector<VkDescriptorSetLayout> sceneDescriptorLayouts = {
               vs->descriptorLayouts.scene,
               vs->descriptorLayouts.material,
               vs->descriptorLayouts.node,
               vs->descriptorLayouts.materialBuffer,
        };

        std::vector<VkDescriptorSetLayout> pickerDescriptorLayouts = {
             vs->descriptorLayouts.picker,
             vs->descriptorLayouts.node,
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
        VkCR(vkCreatePipelineLayout(vb->device, &pipelineLayoutInfo, nullptr, &vs->skyboxPipelineLayout));

        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(sceneDescriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = sceneDescriptorLayouts.data();
        VkCR(vkCreatePipelineLayout(vb->device, &pipelineLayoutInfo, nullptr, &vs->scenePipelineLayout));

        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(pickerDescriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = pickerDescriptorLayouts.data();
        VkCR(vkCreatePipelineLayout(vb->device, &pipelineLayoutInfo, nullptr, &vs->pickerPipelineLayout));

        /* PIPELINE */
        VkSampleCountFlagBits sampleFlag = vb->settings.sampleCount;

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

            vs->pipelines.skybox = pipelineBuilder.Build(vs->skyboxPipelineLayout, &vb->swapchain.colorTarget.format);
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

            vs->pipelines.pbr = pipelineBuilder.Build(vs->scenePipelineLayout, &vb->swapchain.colorTarget.format);

            // pbr double sided pipeline
            pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            vs->pipelines.pbrDoubleSided = pipelineBuilder.Build(vs->scenePipelineLayout, &vb->swapchain.colorTarget.format);

            // pbr alpha blending pipeline
            pipelineBuilder.EnableAlphaBlending();
            vs->pipelines.pbrAlphaBlending = pipelineBuilder.Build(vs->scenePipelineLayout, &vb->swapchain.colorTarget.format);
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

            vs->pipelines.unlit = pipelineBuilder.Build(vs->scenePipelineLayout, &vb->swapchain.colorTarget.format);

            // unlit double sided pipeline
            pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            vs->pipelines.unlitDoubleSided = pipelineBuilder.Build(vs->scenePipelineLayout, &vb->swapchain.colorTarget.format);

            // unlit alpha blending pipeline
            pipelineBuilder.EnableAlphaBlending();
            vs->pipelines.unlitAlphaBlending = pipelineBuilder.Build(vs->scenePipelineLayout, &vb->swapchain.colorTarget.format);
        }

        {   // color picker pipeline
            VulkanPipeline::Builder pipelineBuilder;
            pipelineBuilder.SetShaders(pickerVert, pickerFrag);
            pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
            pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            pipelineBuilder.DisableColorBlending();
            pipelineBuilder.SetMultisampling(VK_SAMPLE_COUNT_1_BIT);
            pipelineBuilder.EnableDepthTest();

            vertexInputAttributes = {
               { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos) },
               { 1, 0, VK_FORMAT_R32G32B32A32_UINT, offsetof(Model::Vertex, joint0) },
               { 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, weight0) },
            };

            pipelineBuilder.SetVertexInput(vertexInputBinding, vertexInputAttributes);

            vs->pipelines.picker = pipelineBuilder.Build(vs->pickerPipelineLayout, &vs->picker.colorTarget.format);
        }

        vkDestroyShaderModule(vb->device, skyboxVert, nullptr);
        vkDestroyShaderModule(vb->device, skyboxFrag, nullptr);
        vkDestroyShaderModule(vb->device, pbrVert, nullptr);
        vkDestroyShaderModule(vb->device, pbrMaterialFrag, nullptr);
        vkDestroyShaderModule(vb->device, unlitMaterialFrag, nullptr);
        vkDestroyShaderModule(vb->device, pickerVert, nullptr);
        vkDestroyShaderModule(vb->device, pickerFrag, nullptr);
    }

    void VulkanScene::Draw(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = vb->swapchain.colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { {0.0065f, 0.005f, 0.0055f, 1.0f} };

        VkRenderingAttachmentInfo depthStencilAttachment = {};
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthStencilAttachment.pNext = nullptr;
        depthStencilAttachment.imageView = vb->swapchain.depthTarget.view;
        depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, vb->swapchain.colorTarget.extent.width, vb->swapchain.colorTarget.extent.height };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = &depthStencilAttachment;
        renderInfo.pStencilAttachment = &depthStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        VkViewport viewport{};
        viewport.width = vb->swapchain.colorTarget.extent.width;
        viewport.height = vb->swapchain.colorTarget.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = { vb->swapchain.colorTarget.extent.width,  vb->swapchain.colorTarget.extent.height };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        if (vs->settings.displayBackground)
        {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->skyboxPipelineLayout, 0, 1, &vb->frames[vb->context.currentFrame].skyboxDescriptorSet, 0, nullptr);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->pipelines.skybox);

            vs->skybox.model->Draw(commandBuffer);
        }

        VkDeviceSize vertexOffsets[1] = { 0 };
        uint32_t dynamicOffset = 0;

        uint32_t index = 0;
        for (Entity entity : vs->context.entities)
        {
            auto& prefabComp = entity.GetComponent<PrefabComponent>();
            Ref<Model> model = vs->context.projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertices.buffer, vertexOffsets);

            if (model->indices.buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(commandBuffer, model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            vs->context.boundPipeline = VK_NULL_HANDLE;

            // one dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
            dynamicOffset = index * static_cast<uint32_t>(vs->dynamicUniformAlignment);

            for (auto node : model->nodes) // opaque primitives first
                RenderNode(node, commandBuffer, Material::ALPHAMODE_OPAQUE, dynamicOffset);
            // for (auto node : model->nodes) // alpha masked primitives
            //     RenderNode(node, commandBuffer, Material::ALPHAMODE_MASK, dynamicOffset);
            // for (auto node : model->nodes) // transparent primitives, TODO: Correct depth sorting
            //     RenderNode(node, commandBuffer, Material::ALPHAMODE_BLEND, dynamicOffset);

            index++;
        }

        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanScene::DrawPicking(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {   // offscreen color picking draw

        if (ImGuiLayer::AreEventsBlocked() || !Input::IsMouseButtonPressed(Mouse::ButtonLeft)) return;

        glm::vec2 mousePosition = Input::GetMousePosition();
        int32_t mouseX = static_cast<int32_t>(mousePosition.x);
        int32_t mouseY = static_cast<int32_t>(mousePosition.y);

        VulkanImage::Transition(commandBuffer, vs->picker.colorTarget.image, vs->picker.colorTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        VulkanImage::Transition(commandBuffer, vs->picker.depthTarget.image, vs->picker.depthTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = vs->picker.colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

        VkRenderingAttachmentInfo depthStencilAttachment = {};
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthStencilAttachment.pNext = nullptr;
        depthStencilAttachment.imageView = vs->picker.depthTarget.view;
        depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = { VkOffset2D{ mouseX, mouseY}, { 1, 1 } }; // 1 pixel render region
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = &depthStencilAttachment;
        renderInfo.pStencilAttachment = &depthStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        VkViewport viewport{};
        viewport.width = vb->swapchain.extent.width;
        viewport.height = vb->swapchain.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { vb->swapchain.extent.width, vb->swapchain.extent.height };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->pipelines.picker);

        uint32_t index = 0;
        auto DrawPrimitive = [&](Node* node)
            {
                if (!node->mesh) return;
                for (Primitive* primitive : node->mesh->primitives)
                {
                    if (primitive->material.alphaMode == Material::ALPHAMODE_OPAQUE)
                    {
                        uint32_t dynamicOffsets[2] = {
                            index * static_cast<uint32_t>(vs->pickerUniformAlignment),
                            index * static_cast<uint32_t>(vs->dynamicUniformAlignment)
                        };

                        const std::vector<VkDescriptorSet> descriptorSets = {
                            vs->picker.descriptorSet,
                            node->mesh->uniform.descriptorSet,
                        };

                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->pickerPipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 2, dynamicOffsets);

                        if (primitive->hasIndices)
                            vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
                        else
                            vkCmdDraw(commandBuffer, primitive->vertexCount, 1, 0, 0);
                    }
                }
            };

        for (Entity entity : vs->context.entities)
        {
            auto& prefabComp = entity.GetComponent<PrefabComponent>();
            Ref<Model> model = vs->context.projectAssetPool->GetAsset<Model>(prefabComp.meshHandle);

            VkDeviceSize vertexOffset = 0;
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertices.buffer, &vertexOffset);

            if (model->indices.buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(commandBuffer, model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            // render each node with picking color
            for (auto node : model->nodes)
            {
                DrawPrimitive(node);

                for (auto child : node->children)
                    DrawPrimitive(child);
            }

            index++;
        }

        vkCmdEndRenderingKHR(commandBuffer);

        VulkanScene::GetColorIDAtMousePosition(mouseX, mouseY);
    }

    void VulkanScene::GetColorIDAtMousePosition(int32_t mouseX, int32_t mouseY)
    {
        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageOffset = { mouseX, mouseY, 0 }; // copy from exact pixel position
        copyRegion.imageExtent = { 1, 1, 1 };

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vb->singleUseCommandPool);

        VulkanImage::Transition(commandBuffer, vs->picker.colorTarget.image, vs->picker.colorTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        vkCmdCopyImageToBuffer(commandBuffer, vs->picker.colorTarget.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vs->picker.stagingBuffer.buffer, 1, &copyRegion);

        VulkanCommand::EndSingleTimeCommands(vb->singleUseCommandPool, commandBuffer);

        glm::vec4* pixelData = static_cast<glm::vec4*>(vs->picker.stagingBuffer.mapped);

        uint8_t red = static_cast<uint8_t>(pixelData->r * 255.0f);
        uint8_t green = static_cast<uint8_t>(pixelData->g * 255.0f);
        uint8_t blue = static_cast<uint8_t>(pixelData->b * 255.0f);

        uint32_t colorID = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);

        DK_CORE_WARN("Color ID: {0}, mouse: {1}, {2}", colorID, mouseX, mouseY);

        vs->context.selectedEntityID = colorID;
    }

}
