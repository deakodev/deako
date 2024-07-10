#include "VulkanBase.h"
#include "dkpch.h"

#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommand.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

namespace Deako {

    VkInstance VulkanBase::s_Instance{ VK_NULL_HANDLE };
    std::vector<const char*> VulkanBase::s_Extensions;
    std::vector<const char*> VulkanBase::s_ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
    VkDevice VulkanBase::s_Device{ VK_NULL_HANDLE };
    VkQueue VulkanBase::s_GraphicsQueue;
    VkQueue VulkanBase::s_PresentQueue;
    uint32_t VulkanBase::s_CurrentFrame = 0;
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    bool VulkanBase::s_FramebufferResized = false;
    std::vector<VkSemaphore> VulkanBase::s_ImageAvailableSemaphores;
    std::vector<VkSemaphore> VulkanBase::s_RenderFinishedSemaphores;
    std::vector<VkFence> VulkanBase::s_InFlightFences;
    VulkanSettings VulkanBase::s_Settings;

    void VulkanBase::Init()
    {
        CreateInstance();

        if (s_Settings.validation)
            VulkanDebugMessenger::Create();

        VulkanSwapChain::CreateSurface();
        s_Device = VulkanDevice::Create();
        s_GraphicsQueue = VulkanDevice::GetGraphicsQueue();
        s_PresentQueue = VulkanDevice::GetPresentQueue();
        VulkanSwapChain::Create();
        VulkanRenderPass::Create();
        VulkanBufferPool::CreateDescriptorSetLayout();
        VulkanPipeline::Create();
        VulkanFramebufferPool::Create();
        VulkanCommandPool::Create();
        VulkanTexturePool::CreateTextures();
        VulkanBufferPool::CreateVertexBuffers();
        VulkanBufferPool::CreateIndexBuffer();
        VulkanBufferPool::CreateUniformBuffers();

        s_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        s_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        s_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // so first call to vkWaitForFences() returns immediately since the fence is already signaled

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkResult result = vkCreateSemaphore(s_Device, &semaphoreInfo, nullptr, &s_ImageAvailableSemaphores[i]);
            DK_CORE_ASSERT(!result, "Failed to create image available semaphore!");

            result = vkCreateSemaphore(s_Device, &semaphoreInfo, nullptr, &s_RenderFinishedSemaphores[i]);
            DK_CORE_ASSERT(!result, "Failed to create render finished semaphore!");

            result = vkCreateFence(s_Device, &fenceInfo, nullptr, &s_InFlightFences[i]);
            DK_CORE_ASSERT(!result, "Failed to create in-flight fence!");
        }
    }

    void VulkanBase::Idle()
    {
        vkDeviceWaitIdle(s_Device);
    }

    void VulkanBase::Shutdown()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(s_Device, s_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(s_Device, s_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(s_Device, s_InFlightFences[i], nullptr);
        }

        VulkanBufferPool::CleanUp();
        VulkanTexturePool::CleanUp();
        VulkanCommandPool::CleanUp();
        VulkanFramebufferPool::CleanUp();
        VulkanPipeline::CleanUp();
        VulkanRenderPass::CleanUp();
        VulkanSwapChain::CleanUp();
        VulkanDevice::CleanUp();

        if (s_Settings.validation)
            VulkanDebugMessenger::CleanUp();

        vkDestroyInstance(s_Instance, nullptr);
    }

    void VulkanBase::CreateInstance()
    {
        #if defined(VK_VALIDATION)
        s_Settings.validation = true;
        #endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Deako Editor";
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
        createInfo.enabledExtensionCount = static_cast<uint32_t>(s_Extensions.size());
        createInfo.ppEnabledExtensionNames = s_Extensions.data();

        if (s_Settings.validation && AreValidationsAvailable())
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = s_ValidationLayers.data();

            // To debug the creation/destruction of the instance without needing a instance to refer to
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            VulkanDebugMessenger::PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &s_Instance);
        DK_CORE_ASSERT(!result, "Failed to create vulkan instance!");
    }

    void VulkanBase::DetermineExtensions()
    {
        // Required extensions - GLFW handy built-in func to get extensions required for our driver
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions; // pointer to a char pointer
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (uint32_t i = 0; i < glfwExtensionCount; i++)
            s_Extensions.emplace_back(glfwExtensions[i]);

        #ifdef VK_USE_PLATFORM_MACOS_MVK
        s_Extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        s_Extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        #endif

        // Required for debug messenger
        if (s_Settings.validation)
            s_Extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Check if extensions are supported
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

        for (const char* requiredExtension : s_Extensions)
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

            DK_CORE_ASSERT(supported, "Required Vk extensions are not supported!");
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
        for (const char* validationLayers : s_ValidationLayers)
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

    void VulkanBase::DrawFrame()
    {
        VkSwapchainKHR swapChain = VulkanSwapChain::GetSwapChain();

        VkSemaphore imageAvailableSemaphore = s_ImageAvailableSemaphores[s_CurrentFrame];
        VkSemaphore renderFinishedSemaphore = s_RenderFinishedSemaphores[s_CurrentFrame];
        VkFence inFlightFence = s_InFlightFences[s_CurrentFrame];

        // (1) wait until previous frame is finished, so command buffer/semaphores are available to use. Waits on host for either any or all of the fences to be signaled before returning. VK_TRUE indicates to wait for all fences, last param is timeout that we set to the max value of UINT64_MAX, which effectively disables timeout
        vkWaitForFences(s_Device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

        // (2) acquire an image from the swap chain
        // third param specifies timeout in nanosecs for an image to become available. Max value of UINT64_MAX, which disables timeout, next two params specify synchronization
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(s_Device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            VulkanSwapChain::Recreate();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            DK_CORE_ASSERT(false, "Failed to acquire swap chain image!");
        }

        VulkanBufferPool::UpdateUniformBuffer(s_CurrentFrame);

        // after acquiring image (to avoid deadlock), manually reset the fence to the unsignaled state
        vkResetFences(s_Device, 1, &inFlightFence);

        // (3) record a command buffer which draws the scene onto that image
        VkCommandBuffer commandBuffer = VulkanCommandPool::Record(s_CurrentFrame, imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer; // the recorded command buffer

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // (4) submit info with recorded command buffer
        result = vkQueueSubmit(s_GraphicsQueue, 1, &submitInfo, inFlightFence);
        DK_CORE_ASSERT(!result, "Failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        // (5) present the swap chain image
        result = vkQueuePresentKHR(s_PresentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || s_FramebufferResized)
        {
            s_FramebufferResized = false;
            VulkanSwapChain::Recreate();
        }
        else if (result != VK_SUCCESS)
        {
            DK_CORE_ASSERT(false, "Failed to present swap chain image!");
        }

        // Set current frame for the next frame
        s_CurrentFrame = (s_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

}
