#pragma once

#include "VulkanDebug.h"
#include "VulkanResource.h"
#include "VulkanDescriptor.h"

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
        VkCommandPool                           commandPool;
        VkCommandBuffer                         commandBuffer;

        VkDescriptorSet                         sceneDescriptorSet;
        VkDescriptorSet                         skyboxDescriptorSet;
        VulkanDescriptor::AllocatorGrowable     descriptorAllocator;

        VkSemaphore                             renderSemaphore;
        VkSemaphore                             presentSemaphore;
        VkFence                                 waitFence;
    };

    struct VulkanBaseSettings
    {
        uint32_t                                frameOverlap = 2;
        bool                                    validationEnabled{ true };
        bool                                    vsync{ false };
        bool                                    multiSampling{ true };
    };

    struct VulkanBaseContext
    {
        std::vector<FrameData>                  frames;
        uint32_t                                currentFrame{ 0 };
    };

    struct VulkanBaseResources
    {
        VkInstance                              instance{ VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT                debugMessenger{ VK_NULL_HANDLE };
        VkSurfaceKHR                            surface;
        VkDevice                                device{ VK_NULL_HANDLE };
        VkPhysicalDevice                        physicalDevice{ VK_NULL_HANDLE };
        VkQueue                                 graphicsQueue{ VK_NULL_HANDLE };
        VkQueue                                 presentQueue{ VK_NULL_HANDLE };
        std::optional<uint32_t>                 graphicsFamily;
        std::optional<uint32_t>                 presentFamily;

        VkPipelineCache                         pipelineCache;
        VkCommandPool                           singleUseCommandPool;
        VkDescriptorPool                        imguiDescriptorPool{ VK_NULL_HANDLE };

        struct
        {
            VulkanImage::AllocatedImage         color;
            VulkanImage::AllocatedImage         depth;
        } multisampleTarget;

        struct
        {
            VkSwapchainKHR                      swapchain{ VK_NULL_HANDLE };
            VulkanSwapchain::SwapchainDetails   details;
            VkFormat                            format;
            VkExtent2D                          extent;
            std::vector<VkImage>                images;
            std::vector<VkImageView>            views;
            VulkanImage::AllocatedImage         colorTarget; // resolves to sc image
            uint32_t                            imageCount;
        } swapchain;

        struct
        {
            VkSampler                                        sampler{ VK_NULL_HANDLE };
            VkFormat                                         format;
            std::vector<VulkanImage::AllocatedImage>         images;
            std::vector<VkDescriptorSet>                     textureIDs;
        } viewport;
    };

    class VulkanBase
    {
    public:
        static void Init();
        static void Idle();
        static void Shutdown();

        static void Draw();
        static void DrawImGui(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        static void Render(Ref<EditorCamera> camera);

        static void WindowResize();

        static Ref<VulkanBaseSettings> GetSettings() { return vbs; }
        static Ref<VulkanBaseContext> GetContext() { return vbc; }
        static Ref<VulkanBaseResources> GetResources() { return vbr; }

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
        static Ref<VulkanBaseSettings> vbs;
        static Ref<VulkanBaseContext> vbc;
        static Ref<VulkanBaseResources> vbr;
    };

}
