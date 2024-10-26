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
            glm::mat4* model{ nullptr };
        } uniformDynamicData;

        struct
        {   // per-object uniform data
            glm::vec4* colorID{ nullptr };
        } uniformPickerData;

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
            Ref<Scene>                                 scene;
            std::vector<Entity>                        entities;
            Ref<ProjectAssetPool>                      projectAssetPool;
            VkPipeline                                 boundPipeline;
            uint32_t                                   selectedEntityID;
        } context;
    };

    class VulkanScene
    {
    public:
        static void Build();
        static void Rebuild();
        static void CleanUp();

        static void Draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        static void DrawPicking(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        static void UpdateUniforms();

        static bool IsInvalid() { return !vs->context.scene->isValid; }
        static void Invalidate() { vs->context.scene->isValid = false; };

        static void GetColorIDAtMousePosition(int32_t mouseX, int32_t mouseY);

        static Ref<VulkanSceneResources> GetResources() { return vs; }

        static uint32_t GetSelectedEntityID() { return vs->context.selectedEntityID; }

    private:
        static void SetUpAssets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipelines();

    private:
        static Ref<VulkanSceneResources> vs;
    };

}
