#pragma once

#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Asset/Scene/Entity.h"
#include "Deako/Renderer/EditorCamera.h"

#include "VulkanResource.h"
#include "VulkanDescriptor.h"
#include "VulkanTexture.h"
#include "VulkanModel.h"

#include <vulkan/vulkan.h>

namespace Deako {

    struct UniformBuffer
    {
        VulkanBuffer::AllocatedBuffer              buffer;
        VkDescriptorBufferInfo                     descriptor;
    };

    struct UniformSet
    {
        UniformBuffer                              dynamic;
        UniformBuffer                              shared;
        UniformBuffer                              light;
    };

    struct VulkanSceneSettings
    {
        bool                                       displayBackground{ true };
        bool                                       animationPaused{ false };
        size_t                                     dynamicUniformAlignment{ 0 };
    };

    struct VulkanSceneContext
    {
        bool                                       sceneValid{ false };
        std::vector<Entity>                        entities;
        Ref<ProjectAssetPool>                      projectAssetPool;
        VkPipeline                                 boundPipeline{ VK_NULL_HANDLE };
    };

    struct VulkanSceneResources
    {
        std::vector<UniformSet>                    uniforms;

        struct
        {   // per-object uniform data
            glm::mat4* model{ nullptr };
        } uniformDynamicData;

        struct
        {   // per-scene uniform data
            glm::mat4                              projection{ 1.0f };
            glm::mat4                              view{ 1.0f };
            glm::vec3                              camPos{ 0.0f };
        } uniformSharedData;

        struct
        {   // light params
            glm::vec3                              color = glm::vec3(1.0f);
            glm::vec3                              rotation = glm::vec3(1.309f, -0.698f, 0.0f);
            Ref<Texture2D>                         lutBrdf;
        } lightSource;

        struct
        {   // light uniform data
            glm::vec4                              lightDir;
            float                                   exposure = 4.5f;
            float                                   gamma = 2.2f;
            float                                   prefilteredCubeMipLevels;
            float                                   scaleIBLAmbient = 1.0f;
            float                                   debugViewInputs = 0;
            float                                   debugViewEquation = 0;
        } uniformLightData;

        VulkanDescriptor::AllocatorGrowable        staticDescriptorAllocator; // vs per-frame

        struct
        {
            VkDescriptorSetLayout                  scene{ VK_NULL_HANDLE };
            VkDescriptorSetLayout                  material{ VK_NULL_HANDLE };
            VkDescriptorSetLayout                  node{ VK_NULL_HANDLE };
            VkDescriptorSetLayout                  materialBuffer{ VK_NULL_HANDLE };
            VkDescriptorSetLayout                  skybox{ VK_NULL_HANDLE };
        } descriptorLayouts;

        VkPipelineLayout                           scenePipelineLayout{ VK_NULL_HANDLE };
        VkPipelineLayout                           skyboxPipelineLayout{ VK_NULL_HANDLE };

        struct
        {
            VkPipeline                             skybox;
            VkPipeline                             pbr;
            VkPipeline                             pbrDoubleSided;
            VkPipeline                             pbrAlphaBlending;
            VkPipeline                             unlit;
            VkPipeline                             unlitDoubleSided;
            VkPipeline                             unlitAlphaBlending;
        } pipelines;

        struct
        {
            Ref<Model>                             model;
            Ref<TextureCubeMap>                    environmentCube{ CreateRef<TextureCubeMap>() };
            Ref<TextureCubeMap>                    irradianceCube{ CreateRef<TextureCubeMap>(TextureCubeMap::IRRADIANCE) };
            Ref<TextureCubeMap>                    prefilteredCube{ CreateRef<TextureCubeMap>(TextureCubeMap::PREFILTERED) };
        } skybox;

        struct
        {   // shader storage buffer object (ssbo)
            VulkanBuffer::AllocatedBuffer          buffer;
            VkDescriptorBufferInfo                 descriptor;
            VkDescriptorSet                        descriptorSet;
        } materialBuffer;

    };

    class VulkanScene
    {
    public:
        static void Build();
        static void Rebuild();
        static void CleanUp();

        static void Draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        static void UpdateUniforms(Ref<EditorCamera> camera);

        static bool IsInvalid() { return !vsc->sceneValid; }
        static void Invalidate() { vsc->sceneValid = false; };

        static Ref<VulkanSceneSettings> GetSettings() { return vss; }
        static Ref<VulkanSceneContext> GetContext() { return vsc; }
        static Ref<VulkanSceneResources> GetResources() { return vsr; }

    private:
        static void SetUpAssets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipelines();

    private:
        static Ref<VulkanSceneSettings> vss;
        static Ref<VulkanSceneContext> vsc;
        static Ref<VulkanSceneResources> vsr;
    };

}
