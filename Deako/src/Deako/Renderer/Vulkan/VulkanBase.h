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

    struct VulkanBaseResources
    {
        std::vector<FrameData>                  frames;

        VkInstance                              instance;
        VkDebugUtilsMessengerEXT                debugMessenger;
        VkSurfaceKHR                            surface;
        VkDevice                                device;
        VkPhysicalDevice                        physicalDevice;
        VkQueue                                 graphicsQueue;
        VkQueue                                 presentQueue;
        std::optional<DkU32>                 graphicsFamily;
        std::optional<DkU32>                 presentFamily;

        VkPipelineCache                         pipelineCache;
        VkCommandPool                           singleUseCommandPool;
        VkDescriptorPool                        imguiDescriptorPool;

        struct
        {
            VkSwapchainKHR                      swapchain;
            SwapchainDetails                    details;
            VkFormat                            format;
            VkExtent2D                          extent;
            std::vector<VkImage>                images;
            std::vector<VkImageView>            views;
            AllocatedImage                      colorTarget; // resolves to swapchain image
            AllocatedImage                      depthTarget;
            DkU32                            imageCount;
        } swapchain;

        struct
        {
            DkU32                            frameOverlap = 2;
            bool                                validationEnabled{ true };
            bool                                vsync{ false };
            VkSampleCountFlagBits               sampleCount{ VK_SAMPLE_COUNT_4_BIT };
        } settings;

        struct
        {
            DkU32                            currentFrame{ 0 };
        } context;
    };

    class VulkanBase
    {
    public:
        static void Init();
        static void Idle();
        static void Shutdown();

        static void Draw();
        static void DrawGui(VkCommandBuffer commandBuffer, DkU32 imageIndex);

        static void RecreateSwapchain();

        static Ref<VulkanBaseResources> GetResources() { return vb; }

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
        static Ref<VulkanBaseResources> vb;
    };

}
