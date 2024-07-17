#include "VulkanBase.h"
#include "dkpch.h"

#include "Deako/Renderer/AssetManager.h"

#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommand.h"
#include "VulkanBuffer.h"
#include "VulkanDepth.h"
#include "VulkanTexture.h"
#include "VulkanViewport.h"
#include "VulkanSync.h"

namespace Deako {

    Ref<VulkanSettings> VulkanBase::s_Settings = CreateRef<VulkanSettings>();
    Ref<VulkanResources> VulkanBase::s_Resources = CreateRef<VulkanResources>();
    VulkanState VulkanBase::s_State;

    static void LoadAssets()
    {
        const std::vector<std::string>& modelPaths = AssetManager::GetModelPaths();
        for (auto& modelPath : modelPaths)
            Model::LoadFromFile(modelPath);

        const std::vector<std::string>& texturePaths = AssetManager::GetTexture2DPaths();
        for (auto& texturePath : texturePaths)
            Texture2D::LoadFromFile(texturePath, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    }

    void VulkanBase::Init(const char* appName)
    {
        CreateInstance(appName);

        if (s_Settings->validation)
            DebugMessenger::Create();

        SwapChain::CreateSurface(); // Surface needed to find supported queue families in next step
        Device::Create(); // Selects phyical, then creates logical
        SwapChain::Create(); // Creates the swap chain images too
        SwapChain::CreateImageViews(); // Just the swap chain image views
        RenderPass::Create();
        BufferPool::CreateDescriptorSetLayout(); // Required before creating pipline
        Pipeline::Create();
        CommandPool::Create(); // Also creates assc buffers
        Viewport::Create(); // Creates images and views
        Depth::CreateAttachment();
        FramebufferPool::CreateFramebuffers();
        TexturePool::CreateSamplers();

        LoadAssets();
        BufferPool::CreateInstanceBuffer();
        BufferPool::CreateUniformBuffers();
        BufferPool::CreateDescriptorPool();
        BufferPool::CreateDescriptorSets();

        Sync::CreateObjects();
    }

    void VulkanBase::Idle()
    {
        vkDeviceWaitIdle(s_Resources->device);
    }

    void VulkanBase::Shutdown()
    {
        Sync::CleanUp();
        BufferPool::CleanUp();
        TexturePool::CleanUp();
        FramebufferPool::CleanUp();
        Depth::CleanUp();
        Viewport::CleanUp();
        CommandPool::CleanUp();
        Pipeline::CleanUp();
        RenderPass::CleanUp();
        SwapChain::CleanUp();
        Device::CleanUp();
        SwapChain::CleanUpSurface();

        if (s_Settings->validation)
            DebugMessenger::CleanUp();

        vkDestroyInstance(s_Resources->instance, nullptr);
    }

    void VulkanBase::CreateInstance(const char* appName)
    {
        #if defined(VK_VALIDATION)
        s_Settings->validation = true;
        #endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Deako Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        #ifdef VK_USE_PLATFORM_MACOS_MVK
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        DetermineExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(s_Settings->instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = s_Settings->instanceExtensions.data();

        if (s_Settings->validation && AreValidationsAvailable())
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(s_Settings->validationLayers.size());
            createInfo.ppEnabledLayerNames = s_Settings->validationLayers.data();
            // Used to debug creation/destruction of the instance without needing a instance to refer to
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            DebugMessenger::PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &s_Resources->instance);
        DK_CORE_ASSERT(!result, "Failed to create vulkan instance!");
    }

    void VulkanBase::DetermineExtensions()
    {
        // Required instanceExtensions - GLFW handy built-in func to get instanceExtensions required for our driver
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions; // pointer to a char pointer
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (uint32_t i = 0; i < glfwExtensionCount; i++)
            s_Settings->instanceExtensions.emplace_back(glfwExtensions[i]);

        #ifdef VK_USE_PLATFORM_MACOS_MVK
        s_Settings->instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        s_Settings->instanceExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        #endif

        // Required for debug messenger
        if (s_Settings->validation)
            s_Settings->instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Check if required instanceExtensions are supported
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

        for (const char* requiredExtension : s_Settings->instanceExtensions)
        {
            bool supported = false;
            for (const auto& supportedExtension : supportedExtensions)
            {
                if (strcmp(requiredExtension, supportedExtension.extensionName) == 0)
                {
                    supported = true;
                    break;
                }
            }
            DK_CORE_ASSERT(supported);
        }
    }

    bool VulkanBase::AreValidationsAvailable()
    {
        // Get list of validation layers available
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Check if each requested validation layer is available
        for (const char* validationLayers : s_Settings->validationLayers)
        {
            bool available = false;
            for (const auto& availableLayer : availableLayers)
            {
                if (strcmp(validationLayers, availableLayer.layerName) == 0)
                {
                    available = true;
                    break;
                }
            }

            if (!available)
                return false;
        }
        return true;
    }

    void VulkanBase::DrawFrame(const glm::mat4& viewProjection)
    {
        VkSemaphore imageAvailableSemaphore = s_Resources->imageAvailableSemaphores[s_State.currentFrame];
        VkSemaphore renderFinishedSemaphore = s_Resources->renderFinishedSemaphores[s_State.currentFrame];
        VkFence inFlightFence = s_Resources->inFlightFences[s_State.currentFrame];

        // (1) wait until previous frame is finished, so command buffer/semaphores are available to use. Waits on host for either any or all of the fences to be signaled before returning. VK_TRUE indicates to wait for all fences, last param is timeout that we set to the max value of UINT64_MAX, which effectively disables timeout
        vkWaitForFences(s_Resources->device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

        // (2) acquire an image from the swap chain
        // third param specifies timeout in nanosecs for an image to become available. Max value of UINT64_MAX, which disables timeout, next two params specify synchronization
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(s_Resources->device, s_Resources->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            SwapChain::Recreate();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            DK_CORE_ASSERT(false);
        }

        // after acquiring image (to avoid deadlock), manually reset the fence to the unsignaled state
        vkResetFences(s_Resources->device, 1, &inFlightFence);

        // (3) record a command buffer which draws the scene onto that image
        CommandPool::RecordImGuiCommandBuffer(s_Resources->imguiCommandBuffers[s_State.currentFrame], imageIndex);
        CommandPool::RecordViewportCommandBuffer(s_Resources->viewportCommandBuffers[s_State.currentFrame], s_State.currentFrame, imageIndex);

        BufferPool::UpdateUniformBuffer(viewProjection);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        std::array<VkCommandBuffer, 2> submitCommandBuffers = {
            s_Resources->imguiCommandBuffers[s_State.currentFrame],
            s_Resources->viewportCommandBuffers[s_State.currentFrame],
        };
        submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());;
        submitInfo.pCommandBuffers = submitCommandBuffers.data(); // the recorded command buffers

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // (4) submit info with recorded command buffer
        result = vkQueueSubmit(s_Resources->graphicsQueue, 1, &submitInfo, inFlightFence);
        DK_CORE_ASSERT(!result);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { s_Resources->swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        // (5) present the swap chain image
        result = vkQueuePresentKHR(s_Resources->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || s_State.framebufferResized)
        {
            s_State.framebufferResized = false;
            SwapChain::Recreate();
        }
        else if (result != VK_SUCCESS)
        {
            DK_CORE_ASSERT(false);
        }

        // Set current frame for the next frame
        s_State.currentFrame = (s_State.currentFrame + 1) % s_Settings->imageCount;
    }

}
