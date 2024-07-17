#pragma once

#include "VulkanUtils.h"
#include "VulkanDepth.h"
#include "VulkanInitializers.h"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace Deako {

    struct VulkanSettings
    {
        bool                               validation{ false };
        std::vector<const char*>           instanceExtensions;
        std::vector<const char*>           deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset" };
        std::vector<const char*>           validationLayers{ "VK_LAYER_KHRONOS_validation" };
        uint32_t                           minImageCount{ 2 };
        uint32_t                           imageCount{ 2 }; // previously MAX_FRAMES_IN_FLIGHT
        VkSampleCountFlagBits              MSAASamples{ VK_SAMPLE_COUNT_1_BIT };

        uint16_t                           maxTextures{ 16 };
    };

    struct VulkanResources
    {
        VkInstance                         instance{ VK_NULL_HANDLE };
        VkPhysicalDevice                   physicalDevice{ VK_NULL_HANDLE };
        VkDevice                           device{ VK_NULL_HANDLE };
        std::optional<uint32_t>            graphicsFamily;
        std::optional<uint32_t>            presentFamily;
        VkQueue                            graphicsQueue{ VK_NULL_HANDLE };
        VkQueue                            presentQueue{ VK_NULL_HANDLE };

        VkFormat                           imageFormat;
        VkExtent2D                         imageExtent;

        VkDescriptorPool                   descriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout              descriptorSetLayout{ VK_NULL_HANDLE };
        VkPipelineLayout                   pipelineLayout{ VK_NULL_HANDLE };

        // Swapchain
        VkSurfaceKHR                       surface{ VK_NULL_HANDLE };
        VkSwapchainKHR                     swapChain{ VK_NULL_HANDLE };
        std::vector<VkImage>               swapChainImages;
        std::vector<VkImageView>           swapChainImageViews;

        // Viewport
        VkRenderPass                       viewportRenderPass{ VK_NULL_HANDLE };
        VkPipeline                         viewportPipeline{ VK_NULL_HANDLE };
        VkCommandPool                      viewportCommandPool{ VK_NULL_HANDLE };
        std::vector<VkCommandBuffer>       viewportCommandBuffers;
        std::vector<VkFramebuffer>         viewportFramebuffers;
        std::vector<VkImage>               viewportImages;
        std::vector<VkDeviceMemory>        viewportImageMemory;
        std::vector<VkImageView>           viewportImageViews;

        // ImGui
        VkRenderPass                       imguiRenderPass{ VK_NULL_HANDLE };
        VkPipeline                         imguiPipeline{ VK_NULL_HANDLE };
        VkCommandPool                      imguiCommandPool{ VK_NULL_HANDLE };
        std::vector<VkCommandBuffer>       imguiCommandBuffers;
        std::vector<VkFramebuffer>         imguiFramebuffers;

        // Depth Attachment
        VkImage                            depthImage{ VK_NULL_HANDLE };
        VkImageView                        depthImageView{ VK_NULL_HANDLE };
        VkDeviceMemory                     depthImageMemory{ VK_NULL_HANDLE };

        // Sync
        std::vector<VkSemaphore>           imageAvailableSemaphores;
        std::vector<VkSemaphore>           renderFinishedSemaphores;
        std::vector<VkFence>               inFlightFences;
    };

    struct VulkanState
    {
        uint32_t                           currentFrame{ 0 };
        bool                               framebufferResized{ false };
    };

    class VulkanBase
    {
    public:
        static void Init(const char* appName);
        static void Idle();
        static void Shutdown();

        static Ref<VulkanSettings>& GetSettings() { return s_Settings; }
        static Ref<VulkanResources>& GetResources() { return s_Resources; }
        static VulkanState* GetState() { return &s_State; }
        static uint32_t GetCurrentFrame() { return s_State.currentFrame; }
        static float GetAspectRatio() { return s_Resources->imageExtent.width / s_Resources->imageExtent.height; }

        static void DrawFrame(const glm::mat4& viewProjection);

    private:
        static void CreateInstance(const char* appName);
        static void DetermineExtensions();
        static bool AreValidationsAvailable();

    private:
        static Ref<VulkanSettings>         s_Settings;
        static Ref<VulkanResources>        s_Resources;
        static VulkanState                 s_State;
    };

}
