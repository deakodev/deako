#pragma once

#include "VulkanTexture.h"
#include "VulkanDepth.h"

#include <vulkan/vulkan.h>

namespace Deako {

    struct VulkanSettings
    {
        bool validation = false;
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
        VkCommandPool                      commandPool{ VK_NULL_HANDLE };
        VkDescriptorPool                   descriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout              descriptorSetLayout{ VK_NULL_HANDLE };
        VkPipeline                         graphicsPipeline{ VK_NULL_HANDLE };
        VkPipelineLayout                   pipelineLayout{ VK_NULL_HANDLE };
        VkRenderPass                       renderPass{ VK_NULL_HANDLE };
        VkSwapchainKHR                     swapChain{ VK_NULL_HANDLE };
        VkSurfaceKHR                       surface{ VK_NULL_HANDLE };
        VkFormat                           imageFormat;
        VkExtent2D                         imageExtent;

        // Viewport
        VkRenderPass                       viewportRenderPass{ VK_NULL_HANDLE };
        VkPipeline                         viewportPipeline{ VK_NULL_HANDLE };
        VkCommandPool                      viewportCommandPool{ VK_NULL_HANDLE };
        std::vector<VkImage>               viewportImages;
        std::vector<VkDeviceMemory>        viewportImageMemory;
        std::vector<VkImageView>           viewportImageViews;

        Scope<DepthAttachment>             depthAttachment;

        uint32_t                           minImageCount{ 2 };
        uint32_t                           imageCount{ 2 }; // previously MAX_FRAMES_IN_FLIGHT
        VkSampleCountFlagBits              MSAASamples{ VK_SAMPLE_COUNT_1_BIT };
    };

    class VulkanBase
    {
    public:
        static void Init();
        static void Idle();
        static void Shutdown();

        static VulkanResources* GetResources() { return &s_Resources; }
        static uint32_t GetCurrentFrame() { return s_CurrentFrame; }
        static const std::vector<const char*>& GetValidations() { return s_ValidationLayers; };

        static bool ValidationsEnabled() { return s_Settings.validation; }

        static void DrawFrame();

    private:
        static void CreateInstance();
        static void DetermineExtensions();
        static bool AreValidationsAvailable();

    private:
        static std::vector<const char*> s_Extensions;
        static std::vector<const char*> s_ValidationLayers;

        static uint32_t s_CurrentFrame;
        static bool s_FramebufferResized;

        static std::vector<VkSemaphore> s_ImageAvailableSemaphores;
        static std::vector<VkSemaphore> s_RenderFinishedSemaphores;
        static std::vector<VkFence> s_InFlightFences;

        static VulkanSettings s_Settings;
        static VulkanResources s_Resources;
    };

}
