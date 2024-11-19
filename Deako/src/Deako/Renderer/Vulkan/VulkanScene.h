#pragma once

#include "Deako/Asset/AssetManager.h"
#include "Deako/Asset/Pool/ProjectAssetPool.h"
#include "Deako/Asset/Scene/Scene.h"
#include "Deako/Renderer/EditorCamera.h"

#include "VulkanResource.h"
#include "VulkanDescriptor.h"
#include "VulkanTexture.h"
#include "VulkanModel.h"

#include <vulkan/vulkan.h>

namespace Deako {

    struct UniformSet
    {
        UniformBuffer                              dynamic;
        UniformBuffer                              shared;
        UniformBuffer                              light;
    };

    struct VulkanSceneResources
    {
        std::vector<UniformSet>                    uniforms;
        size_t                                     dynamicUniformAlignment{ 0 };

        struct
        {   // per-object uniform data
            DkMat4* model{ nullptr };
        } uniformDynamicData;

        struct
        {   // per-scene uniform data
            DkMat4                                 projection{ 1.0f };
            DkMat4                                 view{ 1.0f };
            DkVec3                                 camPos{ 0.0f };
        } uniformSharedData;

        struct
        {   // light params
            DkVec3                                 color = DkVec3(1.0f);
            DkVec3                                 rotation = DkVec3(1.309f, -0.698f, 0.0f);
            Ref<Texture2D>                         lutBrdf;
        } lightSource;

        struct
        {   // light uniform data
            DkVec4                                 lightDir;
            DkF32                                  exposure = 4.5f;
            DkF32                                  gamma = 2.2f;
            DkF32                                  prefilteredCubeMipLevels;
            DkF32                                  scaleIBLAmbient = 1.0f;
            DkF32                                  debugViewInputs = 0;
            DkF32                                  debugViewEquation = 0;
        } uniformLightData;

        VulkanDescriptor::AllocatorGrowable        staticDescriptorAllocator; // vs per-frame

        struct
        {
            VkDescriptorSetLayout                  scene;
            VkDescriptorSetLayout                  material;
            VkDescriptorSetLayout                  node;
            VkDescriptorSetLayout                  materialBuffer;
            VkDescriptorSetLayout                  skybox;
        } descriptorLayouts;

        VkPipeline                                 boundPipeline;
        VkPipelineLayout                           scenePipelineLayout;
        VkPipelineLayout                           skyboxPipelineLayout;

        struct
        {
            VkPipeline                             skybox;
            VkPipeline                             pbr;
            VkPipeline                             pbrDoubleSided;
            VkPipeline                             pbrAlphaBlending;
            VkPipeline                             unlit;
            VkPipeline                             unlitDoubleSided;
            VkPipeline                             unlitAlphaBlending;
            VkPipeline                             outline;
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
            AllocatedBuffer                        buffer;
            VkDescriptorBufferInfo                 descriptor;
            VkDescriptorSet                        descriptorSet;
        } materialBuffer;
    };

    Scene& GetActiveScene();

    class VulkanScene
    {
    public:
        VulkanScene(Scene* scene);

        void Build();
        void Rebuild();
        void CleanUp();

        void OnUpdate();
        void Draw(VkCommandBuffer commandBuffer, DkU32 imageIndex);

        VulkanSceneResources& GetResources() { return m_Resources; }
        static VulkanSceneResources& GetActiveSceneResources() { return GetActiveScene().GetVulkanScene().GetResources(); }

    private:
        void SetUpAssets();
        void SetUpUniforms();
        void SetUpDescriptors();
        void SetUpPipelines();

    private:
        VulkanSceneResources m_Resources;

        bool m_DisplayBackground{ true };

        Scene* m_ParentScene; // non-owning pointer to parent scene
    };

}
