#pragma once

#include "VulkanDebug.h"
#include "VulkanModel.h"
#include "VulkanTexture.h"
#include "VulkanTypes.h"
#include "VulkanUtils.h"
#include "VulkanScene.h"
#include "VulkanDescriptor.h"

#include "Deako/Asset/Scene/Entity.h"
#include "Deako/Renderer/EditorCamera.h"

#include <vulkan/vulkan.h>

namespace Deako {

    // extension functions since macos doesn't support vk 1.3
    extern PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR;
    extern PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR;
    extern PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
    extern PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;
    extern PFN_vkCmdBlitImage2KHR vkCmdBlitImage2KHR;

    struct FrameData
    {
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        VulkanDescriptor::AllocatorGrowable descriptorAllocator;
        VkDescriptorSet sceneDescriptorSet;
        VkDescriptorSet skyboxDescriptorSet;

        VkSemaphore renderSemaphore;
        VkSemaphore presentSemaphore;
        VkFence waitFence;
    };

    struct VulkanSettings
    {
        bool                               validationEnabled{ true };
        bool                               vsync{ false };
        bool                               multiSampling{ true };
        bool                               displayBackground{ true };
        bool                               animationPaused{ false };

        uint32_t                           frameOverlap = 2;
    };

    struct VulkanContext
    {
        std::vector<FrameData>             frames;
        uint32_t                           currentFrame{ 0 };
        VkPipeline                         boundPipeline{ VK_NULL_HANDLE };
    };

    struct VulkanResources
    {
        VkInstance                         instance{ VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT           debugMessenger{ VK_NULL_HANDLE };
        VkSurfaceKHR                       surface;
        VkDevice                           device{ VK_NULL_HANDLE };
        VkPhysicalDevice                   physicalDevice{ VK_NULL_HANDLE };
        VkQueue                            graphicsQueue{ VK_NULL_HANDLE };
        VkQueue                            presentQueue{ VK_NULL_HANDLE };
        std::optional<uint32_t>            graphicsFamily;
        std::optional<uint32_t>            presentFamily;

        VkPipelineCache                    pipelineCache;
        VkCommandPool                      singleUseCommandPool;
        VkDescriptorPool                   imguiDescriptorPool{ VK_NULL_HANDLE };

        struct
        {
            AllocatedImage                 color;
            AllocatedImage                 depth;
        } multisampleTarget;

        struct
        {
            VkSwapchainKHR                 swapchain{ VK_NULL_HANDLE };
            SwapchainDetails               details;
            VkFormat                       format;
            VkExtent2D                     extent;
            std::vector<VkImage>           images;
            std::vector<VkImageView>       views;
            AllocatedImage                 colorTarget; // resolves to sc image
            uint32_t                       imageCount;
        } swapchain;

        struct
        {
            VkSampler                      sampler{ VK_NULL_HANDLE };
            VkFormat                       format;
            std::vector<AllocatedImage>    images;
            std::vector<VkDescriptorSet>   textureIDs;
        } viewport;




        struct Skybox
        {
            Ref<Model> model;
            Ref<TextureCubeMap> environmentCube{ CreateRef<TextureCubeMap>() };
            Ref<TextureCubeMap> irradianceCube{ CreateRef<TextureCubeMap>(TextureCubeMap::IRRADIANCE) };
            Ref<TextureCubeMap> prefilteredCube{ CreateRef<TextureCubeMap>(TextureCubeMap::PREFILTERED) };
        } skybox;

        std::vector<Entity> entities;

        struct UniformSet
        {
            UniformBuffer dynamic;
            UniformBuffer shared;
            UniformBuffer light;
        };

        struct
        {
            AllocatedBuffer buffer;
            VkDescriptorBufferInfo descriptor;
            VkDescriptorSet descriptorSet;
        } materialBuffer; // shader storage buffer object (ssbo)

        std::vector<UniformSet>            uniforms;

        size_t                             dynamicUniformAlignment{ 0 };

        struct // per-object uniform data
        {
            glm::mat4* model{ nullptr };
        } uniformDynamicData;

        struct // per-scene uniform data
        {
            glm::mat4 projection{ 1.0f };
            glm::mat4 view{ 1.0f };
            glm::vec3 camPos{ 0.0f };
        } uniformSharedData;

        struct // light uniform data
        {
            glm::vec4 lightDir;
            float exposure = 4.5f;
            float gamma = 2.2f;
            float prefilteredCubeMipLevels;
            float scaleIBLAmbient = 1.0f;
            float debugViewInputs = 0;
            float debugViewEquation = 0;
        } uniformLightData;

        struct LightSource
        {
            glm::vec3 color = glm::vec3(1.0f);
            glm::vec3 rotation = glm::vec3(1.309f, -0.698f, 0.0f);
        } lightSource;

        struct DescriptorSetLayouts
        {
            VkDescriptorSetLayout scene{ VK_NULL_HANDLE };
            VkDescriptorSetLayout material{ VK_NULL_HANDLE };
            VkDescriptorSetLayout node{ VK_NULL_HANDLE };
            VkDescriptorSetLayout materialBuffer{ VK_NULL_HANDLE };
            VkDescriptorSetLayout skybox{ VK_NULL_HANDLE };
        } descriptorLayouts;

        VkPipeline skyboxPipeline;
        VkPipeline pbrPipeline;
        VkPipeline pbrDoubleSidedPipeline;
        VkPipeline pbrAlphaBlendingPipeline;
        VkPipeline unlitPipeline;
        VkPipeline unlitDoubleSidedPipeline;
        VkPipeline unlitAlphaBlendingPipeline;

        VulkanDescriptor::AllocatorGrowable        staticDescriptorAllocator; // vs per-frame

        VkPipelineLayout                   scenePipelineLayout{ VK_NULL_HANDLE };
        VkPipelineLayout                   skyboxPipelineLayout{ VK_NULL_HANDLE };

        struct
        {
            Ref<Texture2D> lutBrdf;
        } textures;
    };

    class VulkanBase
    {
    public:
        static void Init();
        static void Idle();
        static void Shutdown();

        static void Render(Ref<EditorCamera> camera);

        static void Draw();
        static void DrawImGui(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        static void WindowResize();

        static Ref<VulkanSettings>& GetSettings() { return vs; }
        static Ref<VulkanContext>& GetContext() { return vc; }
        static Ref<VulkanResources>& GetResources() { return vr; }

    private:
        static void CreateInstance();
        static void SetUpDebugMessenger();
        static void SetUpDevice();
        static void SetUpSwapchain();
        static void SetUpPipelineCache();
        static void SetUpCommands();
        static void SetUpSyncObjects();
        static void SetUpImGui();

    private:
        static Ref<VulkanSettings>         vs;
        static Ref<VulkanContext>          vc;
        static Ref<VulkanResources>        vr;
    };

}
