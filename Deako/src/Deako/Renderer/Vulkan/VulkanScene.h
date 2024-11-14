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

    struct UniformSet
    {
        UniformBuffer                              dynamic;
        UniformBuffer                              shared;
        UniformBuffer                              light;
        UniformBuffer                              picker;
    };

    struct VulkanSceneResources
    {
        std::vector<UniformSet>                    uniforms;

        size_t                                     dynamicUniformAlignment{ 0 };
        size_t                                     pickerUniformAlignment{ 0 };

        struct
        {   // per-object uniform data
            DkMat4* model{ nullptr };
        } uniformDynamicData;

        struct
        {   // per-object uniform data
            DkVec4* colorID{ nullptr };
        } uniformPickerData;

        struct
        {   // per-scene uniform data
            DkMat4                              projection{ 1.0f };
            DkMat4                              view{ 1.0f };
            DkVec3                              camPos{ 0.0f };
        } uniformSharedData;

        struct
        {   // light params
            DkVec3                              color = DkVec3(1.0f);
            DkVec3                              rotation = DkVec3(1.309f, -0.698f, 0.0f);
            Ref<Texture2D>                         lutBrdf;
        } lightSource;

        struct
        {   // light uniform data
            DkVec4                              lightDir;
            DkF32                                   exposure = 4.5f;
            DkF32                                   gamma = 2.2f;
            DkF32                                   prefilteredCubeMipLevels;
            DkF32                                   scaleIBLAmbient = 1.0f;
            DkF32                                   debugViewInputs = 0;
            DkF32                                   debugViewEquation = 0;
        } uniformLightData;

        VulkanDescriptor::AllocatorGrowable        staticDescriptorAllocator; // vs per-frame

        struct
        {
            VkDescriptorSetLayout                  scene;
            VkDescriptorSetLayout                  material;
            VkDescriptorSetLayout                  node;
            VkDescriptorSetLayout                  materialBuffer;
            VkDescriptorSetLayout                  skybox;
            VkDescriptorSetLayout                  picker;
        } descriptorLayouts;

        VkPipelineLayout                           scenePipelineLayout;
        VkPipelineLayout                           pickerPipelineLayout;
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
            VkPipeline                             picker;
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

        struct
        {
            AllocatedImage                         colorTarget;
            AllocatedImage                         depthTarget;
            AllocatedBuffer                        stagingBuffer;
            VkDescriptorSet                        descriptorSet;
        } picker;

        struct
        {
            bool                                       displayBackground{ true };
            bool                                       animationPaused{ false };
        } settings;

        struct
        {
            VkPipeline                                 boundPipeline;
        } context;
    };

    class VulkanScene
    {
    public:
        static void Build();
        static void Rebuild();
        static void CleanUp();

        static void OnUpdate();
        static void Draw(VkCommandBuffer commandBuffer, DkU32 imageIndex);

        static Ref<VulkanSceneResources> GetResources() { return vs; }

    private:
        static void SetUpAssets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipelines();

    private:
        static Ref<VulkanSceneResources> vs;
    };

}
