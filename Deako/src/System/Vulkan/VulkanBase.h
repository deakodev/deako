#pragma once

#include "Deako/Renderer/EditorCamera.h"

#include "VulkanDebug.h"
#include "VulkanModel.h"
#include "VulkanTexture.h"
#include "VulkanTypes.h"
#include "VulkanUtils.h"

#include <vulkan/vulkan.h>

namespace Deako {

    // extension functions since macos doesn't support vk 1.3
    extern PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR;
    extern PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR;
    extern PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
    extern PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;
    extern PFN_vkCmdBlitImage2KHR vkCmdBlitImage2KHR;

    struct VulkanSettings
    {
        bool                               validationEnabled{ true };
        bool                               vsync{ false };
        bool                               multiSampling{ true };
        bool                               displayBackground{ true };
        bool                               animationPaused{ false };

        VkSampleCountFlagBits              sampleCount{ VK_SAMPLE_COUNT_4_BIT };
        uint32_t                           frameOverlap = 2;

        std::string                        assetPath{ "Deako-Editor/assets/" };
    };

    struct VulkanResources
    {
        bool                               prepared{ false };
        uint32_t                           currentFrame{ 0 };

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
        VkPipelineLayout                   pipelineLayout{ VK_NULL_HANDLE };
        std::unordered_map<std::string, VkPipeline>         pipelines;
        VkPipeline                                          boundPipeline{ VK_NULL_HANDLE };

        VkCommandPool                      singleUseCommandPool;

        std::vector<FrameData>             frames;

        struct MultisampleTarget
        {
            AllocatedImage                     color;
            AllocatedImage                     depth;
        } multisampleTarget;

        struct Swapchain
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

        struct Viewport
        {
            VkSampler                      sampler{ VK_NULL_HANDLE };
            VkFormat                       format;
            std::vector<AllocatedImage>    images;
            std::vector<VkDescriptorSet>   textureIDs;
        } viewport;

        VkDescriptorPool                   imguiDescriptorPool{ VK_NULL_HANDLE };

        // assets
        struct Textures
        {
            Texture2D empty;
            Texture2D lutBrdf;
            TextureCubeMap environmentCube{ TextureCubeMap::NONE };
            TextureCubeMap irradianceCube{ TextureCubeMap::IRRADIANCE };
            TextureCubeMap prefilteredCube{ TextureCubeMap::PREFILTERED };
        } textures;


        std::unordered_map<std::string, Ref<Model>> models;

        struct Scene
        {
            int32_t animationIndex{ 0 };
            float animationTimer{ 0.0f };
            bool animate{ true };
        } scene;

        struct ShaderValuesMatrices
        {
            glm::mat4 projection{ 1.0f };
            glm::mat4 model{ 1.0f };
            glm::mat4 view{ 1.0f };
            glm::vec3 camPos{ 0.0f };
        } shaderValuesScene, shaderValuesSkybox;

        struct ShaderValuesParams
        {
            glm::vec4 lightDir;
            float exposure = 4.5f;
            float gamma = 2.2f;
            float prefilteredCubeMipLevels;
            float scaleIBLAmbient = 1.0f;
            float debugViewInputs = 0;
            float debugViewEquation = 0;
        } shaderValuesParams;

        struct LightSource
        {
            glm::vec3 color = glm::vec3(1.0f);
            glm::vec3 rotation = glm::vec3(75.0f, -40.0f, 0.0f);
        } lightSource;

        struct UniformSet
        {
            UniformBuffer scene;
            UniformBuffer skybox;
            UniformBuffer params;
        };

        struct DescriptorSetLayouts
        {
            VkDescriptorSetLayout scene{ VK_NULL_HANDLE };
            VkDescriptorSetLayout material{ VK_NULL_HANDLE };
            VkDescriptorSetLayout node{ VK_NULL_HANDLE };
            VkDescriptorSetLayout materialBuffer{ VK_NULL_HANDLE };
        } descriptorSetLayouts;

        struct DescriptorSets
        {
            VkDescriptorSet scene;
            VkDescriptorSet skybox;
        };

        std::map<std::string, std::string> environments;
        AllocatedBuffer                    shaderMaterialBuffer;
        VkDescriptorBufferInfo             shaderMaterialDescriptorInfo{ VK_NULL_HANDLE };
        VkDescriptorSet                    shaderMaterialDescriptorSet{ VK_NULL_HANDLE };

        std::vector<UniformSet>            uniforms;
        std::vector<DescriptorSets>        descriptorSets;
        VkDescriptorPool                   descriptorPool{ VK_NULL_HANDLE };

        Camera                             camera;
    };

    class VulkanBase
    {
    public:
        static void Init(const char* appName);
        static void Idle();
        static void Shutdown();

        static void Render();

        static Ref<VulkanSettings>& GetSettings() { return vs; }
        static Ref<VulkanResources>& GetResources() { return vr; }

        static void ViewportResize(const glm::vec2& viewportSize);

        static void UpdateUniforms();

        static void LoadModel(const std::string& relativePath);

    private:
        static void CreateInstance(const char* appName);
        static void SetUpDebugMessenger();
        static void SetUpDevice();
        static void SetUpSwapchain();
        static void SetUpCommands();
        static void SetUpSyncObjects();
        static void SetUpAssets();
        static void SetUpUniforms();
        static void SetUpDescriptors();
        static void SetUpPipelines();
        static void SetUpImGui();

        static void AddPipelineSet(const std::string prefix, const std::string vertexShader, const std::string fragmentShader);

        static void DrawFrame();
        static void DrawViewport(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        static void DrawImGui(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        // static void UpdateUniforms();
        static void UpdateShaderParams();

        static void WindowResize();

    private:
        static Ref<VulkanResources>        vr;
        static Ref<VulkanSettings>         vs;
    };

}
