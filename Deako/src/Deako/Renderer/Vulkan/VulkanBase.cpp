#include "VulkanBase.h"
#include "dkpch.h"

#include "Deako/Core/Application.h" 
#include "Deako/ImGui/ImGuiLayer.h"

#include "VulkanScene.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Deako {

    PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR = nullptr;
    PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR = nullptr;
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = nullptr;
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = nullptr;

    Ref<VulkanBaseSettings> VulkanBase::vbs = CreateRef<VulkanBaseSettings>();
    Ref<VulkanBaseContext> VulkanBase::vbc = CreateRef<VulkanBaseContext>();
    Ref<VulkanBaseResources> VulkanBase::vbr = CreateRef<VulkanBaseResources>();

    Ref<VulkanSceneSettings> VulkanScene::vss = CreateRef<VulkanSceneSettings>();
    Ref<VulkanSceneContext> VulkanScene::vsc = CreateRef<VulkanSceneContext>();
    Ref<VulkanSceneResources> VulkanScene::vsr = CreateRef<VulkanSceneResources>();

    void VulkanBase::Init()
    {
        CreateInstance();

        SetUpDebugMessenger();

        SetUpDevice();

        SetUpSwapchain();

        SetUpPipelineCache();

        SetUpCommands();

        SetUpSyncObjects();

        SetUpImGui();
    }

    void VulkanBase::Idle()
    {
        vkDeviceWaitIdle(vbr->device);
    }

    void VulkanBase::Shutdown()
    {
        VulkanBase::Idle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(vbr->device, vbr->imguiDescriptorPool, nullptr);
        vkDestroySampler(vbr->device, vbr->viewport.sampler, nullptr);

        for (auto image : vbr->viewport.images)
            VulkanImage::Destroy(image);

        for (auto& frame : vbc->frames)
        {
            vkDestroySemaphore(vbr->device, frame.renderSemaphore, nullptr);
            vkDestroySemaphore(vbr->device, frame.presentSemaphore, nullptr);
            vkDestroyFence(vbr->device, frame.waitFence, nullptr);
            vkDestroyCommandPool(vbr->device, frame.commandPool, nullptr);
        }

        vkDestroyCommandPool(vbr->device, vbr->singleUseCommandPool, nullptr);

        vkDestroyPipelineCache(vbr->device, vbr->pipelineCache, nullptr);

        for (auto imageView : vbr->swapchain.views)
            vkDestroyImageView(vbr->device, imageView, nullptr);

        VulkanImage::Destroy(vbr->swapchain.colorTarget);
        VulkanImage::Destroy(vbr->multisampleTarget.color);
        VulkanImage::Destroy(vbr->multisampleTarget.depth);

        vkDestroySwapchainKHR(vbr->device, vbr->swapchain.swapchain, nullptr);

        vkDestroyDevice(vbr->device, nullptr);

        vkDestroySurfaceKHR(vbr->instance, vbr->surface, nullptr);

        if (vbs->validationEnabled) VulkanDebug::DestroyDebugUtilsMessengerEXT();

        vkDestroyInstance(vbr->instance, nullptr);
    }

    void VulkanBase::CreateInstance()
    {
        #if defined(VK_VALIDATION)
        vbs->validationEnabled = true;
        #endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = Application::Get().GetSpecification().name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Deako Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> instanceExtensions{ glfwExtensions, glfwExtensions + glfwExtensionCount };

        for (uint32_t i = 0; i < glfwExtensionCount; i++)
            instanceExtensions.emplace_back(glfwExtensions[i]);

        #if defined(VK_USE_PLATFORM_MACOS_MVK) // enable surface extensions depending for mac
        instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        instanceExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        #endif
        if (vbs->validationEnabled)
            instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // check if required instanceExtensions are supported
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

        for (const char* requiredExtension : instanceExtensions)
        {
            bool supported = false;
            for (const auto& supportedExtension : supportedExtensions)
            {
                if (strcmp(requiredExtension, supportedExtension.extensionName) == 0)
                {
                    supported = true; break;
                }
            }
            DK_CORE_ASSERT(supported, "Required instance extensions not supported!");
        }

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pNext = nullptr;
        instanceInfo.pApplicationInfo = &appInfo;
        #if defined(VK_USE_PLATFORM_MACOS_MVK)
        instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        if (instanceExtensions.size() > 0)
        {
            instanceInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
            instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
        }

        std::vector<const char*> validationLayerNames;
        if (vbs->validationEnabled)
        {
            validationLayerNames.push_back("VK_LAYER_KHRONOS_validation");
            instanceInfo.enabledLayerCount = (uint32_t)validationLayerNames.size();
            instanceInfo.ppEnabledLayerNames = validationLayerNames.data();
            // used to debug instance creation without needing instance reference
            VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
            VulkanDebug::PopulateDebugMessengerCreateInfo(debugInfo);
            instanceInfo.pNext = &debugInfo;
        }

        VkCR(vkCreateInstance(&instanceInfo, nullptr, &vbr->instance));
    }

    void VulkanBase::SetUpDebugMessenger()
    {
        if (vbs->validationEnabled)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
            VulkanDebug::PopulateDebugMessengerCreateInfo(debugInfo);
            VkCR(VulkanDebug::CreateDebugUtilsMessengerEXT(&debugInfo));
        }
    }

    void VulkanBase::SetUpDevice()
    {
        /* CREATE SURFACE */
        Ref<GLFWwindow> window = Application::Get().GetWindow().GetNativeWindow();
        VkCR(glfwCreateWindowSurface(vbr->instance, window.get(), nullptr, &vbr->surface));

        /* DETERMINE PHYSICAL */
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vbr->instance, &deviceCount, nullptr);
        DK_CORE_ASSERT(deviceCount);  // check if any devices are supported

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(vbr->instance, &deviceCount, physicalDevices.data());

        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& physicalDevice : physicalDevices)
        {   // determine scores of each device
            int score = VulkanDevice::RatePhysical(physicalDevice);
            candidates.insert(std::make_pair(score, physicalDevice));
        }

        // determine best candidate and set as physical device
        if (candidates.rbegin()->first > 0) vbr->physicalDevice = candidates.rbegin()->second;
        else DK_CORE_ASSERT(false);

        // set family indices of our selected physical device
        VulkanDevice::FindQueueFamilies(vbr->physicalDevice);

        /* CREATE LOGICAL */
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // obtain
        std::set<uint32_t> queueFamilies = { vbr->graphicsFamily.value(), vbr->presentFamily.value() };

        float queuePriority = 1.0f; // 0.0 - 1.0, influences scheduling of command buffer
        for (uint32_t queueFamily : queueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures enabledFeatures{};
        enabledFeatures.samplerAnisotropy = VK_TRUE;

        VkPhysicalDeviceDynamicRenderingFeaturesKHR drFeatures{};
        drFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        drFeatures.dynamicRendering = VK_TRUE;

        VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features{};
        sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync2Features.synchronization2 = VK_TRUE;
        sync2Features.pNext = &drFeatures;

        std::vector<const char*> enabledExtensions{};
        enabledExtensions.push_back("VK_KHR_portability_subset");
        enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);

        VulkanDevice::CheckExtensionSupport(vbr->physicalDevice, enabledExtensions);

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
        deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceInfo.pEnabledFeatures = &enabledFeatures;
        deviceInfo.pNext = &sync2Features;

        if (enabledExtensions.size() > 0)
        {
            deviceInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
            deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
        }

        VkCR(vkCreateDevice(vbr->physicalDevice, &deviceInfo, nullptr, &vbr->device));

        /* GET QUEUE FAMILIES */
        vkGetDeviceQueue(vbr->device, vbr->graphicsFamily.value(), 0, &vbr->graphicsQueue);
        vkGetDeviceQueue(vbr->device, vbr->presentFamily.value(), 0, &vbr->presentQueue);

        // find certain extension functions since macos doesn't support 1.3
        vkCmdPipelineBarrier2KHR =
            (PFN_vkCmdPipelineBarrier2KHR)vkGetDeviceProcAddr(vbr->device, "vkCmdPipelineBarrier2KHR");
        vkQueueSubmit2KHR =
            (PFN_vkQueueSubmit2KHR)vkGetDeviceProcAddr(vbr->device, "vkQueueSubmit2KHR");
        if (!vkCmdPipelineBarrier2KHR || !vkQueueSubmit2KHR)
            throw std::runtime_error("Failed to load VK_KHR_synchronization2 functions!");

        vkCmdBeginRenderingKHR =
            (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(vbr->device, "vkCmdBeginRenderingKHR");
        vkCmdEndRenderingKHR =
            (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(vbr->device, "vkCmdEndRenderingKHR");
        if (!vkCmdBeginRenderingKHR || !vkCmdEndRenderingKHR)
            throw std::runtime_error("Failed to load VK_KHR_dynamic_rendering functions!");
    }

    void VulkanBase::SetUpSwapchain()
    {
        /* SWAPCHAIN CREATION */
        VkSwapchainKHR oldSwapchain = vbr->swapchain.swapchain;

        VulkanSwapchain::QuerySupport(vbr->physicalDevice);

        VkSurfaceCapabilitiesKHR caps = vbr->swapchain.details.capabilities;
        std::vector<VkSurfaceFormatKHR> formats = vbr->swapchain.details.formats;
        std::vector<VkPresentModeKHR> presentModes = vbr->swapchain.details.presentModes;

        VkSurfaceFormatKHR surfaceFormat = VulkanSwapchain::ChooseSurfaceFormat(formats);
        vbr->swapchain.format = surfaceFormat.format;

        VkPresentModeKHR presentMode = VulkanSwapchain::ChoosePresentMode(presentModes);
        vbr->swapchain.extent = VulkanSwapchain::ChooseExtent(caps);

        // check to make sure we dont exceed the max number of possible images
        vbr->swapchain.imageCount = caps.minImageCount;
        if (caps.maxImageCount > 0 && vbr->swapchain.imageCount > caps.maxImageCount)
            vbr->swapchain.imageCount = caps.maxImageCount;

        // find the transformation of the surface
        VkSurfaceTransformFlagsKHR preTransform;
        if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;// We prefer a non-rotated transform
        else
            preTransform = caps.currentTransform;

        // find a supported composite alpha format (not all devices support alpha opaque)
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };

        for (auto& compositeAlphaFlag : compositeAlphaFlags)
        {
            if (caps.supportedCompositeAlpha & compositeAlphaFlag)
            {
                compositeAlpha = compositeAlphaFlag; break;
            };
        }

        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = vbr->surface;
        swapchainInfo.minImageCount = vbr->swapchain.imageCount;
        swapchainInfo.imageFormat = vbr->swapchain.format;
        swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainInfo.imageExtent = vbr->swapchain.extent;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.presentMode = presentMode;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.compositeAlpha = compositeAlpha;
        swapchainInfo.oldSwapchain = oldSwapchain;

        if (caps.supportedTransforms & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            swapchainInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        // Enable transfer destination on swap chain images if supported
        if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            swapchainInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        uint32_t queueFamilyIndices[] = { vbr->graphicsFamily.value(), vbr->presentFamily.value() };
        if (vbr->graphicsFamily != vbr->presentFamily)
        {
            swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainInfo.queueFamilyIndexCount = 2;
            swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainInfo.queueFamilyIndexCount = 0;
            swapchainInfo.pQueueFamilyIndices = nullptr;
        }

        VkCR(vkCreateSwapchainKHR(vbr->device, &swapchainInfo, nullptr, &vbr->swapchain.swapchain));

        // if an existing swap chain is re-created, destroy the old swap chain
        if (oldSwapchain != VK_NULL_HANDLE)
        {
            for (auto imageView : vbr->swapchain.views)
                vkDestroyImageView(vbr->device, imageView, nullptr);
            vkDestroySwapchainKHR(vbr->device, oldSwapchain, nullptr);
        }

        /* SWAPCHAIN IMAGES */
        vkGetSwapchainImagesKHR(vbr->device, vbr->swapchain.swapchain, &vbr->swapchain.imageCount, nullptr);
        vbr->swapchain.images.resize(vbr->swapchain.imageCount);
        vkGetSwapchainImagesKHR(vbr->device, vbr->swapchain.swapchain, &vbr->swapchain.imageCount, vbr->swapchain.images.data());

        /* SWAPCHAIN IMAGE VIEWS */
        vbr->swapchain.views.resize(vbr->swapchain.imageCount);
        for (size_t i = 0; i < vbr->swapchain.imageCount; i++)
        {
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.image = vbr->swapchain.images[i];
            imageViewInfo.format = vbr->swapchain.format;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

            VkCR(vkCreateImageView(vbr->device, &imageViewInfo, nullptr, &vbr->swapchain.views[i]));
        }

        /* COLOR TARGET */ // resolves to the sc image view
        VkExtent3D colorExtent = { vbr->swapchain.extent.width, vbr->swapchain.extent.height, 1 };
        VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
        VkSampleCountFlagBits colorSamples = VK_SAMPLE_COUNT_4_BIT;

        VkImageUsageFlags colorUsages{};
        colorUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        colorUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vbr->swapchain.colorTarget =
            VulkanImage::Create(colorExtent, colorFormat, colorSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        /* MULTISAMPLE TARGET */ // resolves to the viewport image view
        vbr->multisampleTarget.color =
            VulkanImage::Create(colorExtent, colorFormat, colorSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        VkExtent3D depthExtent = { vbr->swapchain.extent.width, vbr->swapchain.extent.height, 1 };
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        VkSampleCountFlagBits depthSamples = VK_SAMPLE_COUNT_4_BIT;

        VkImageUsageFlags depthUsages{};
        depthUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vbr->multisampleTarget.depth =
            VulkanImage::Create(depthExtent, depthFormat, depthSamples, depthUsages, 1, VK_IMAGE_TYPE_2D);
    }

    void VulkanBase::SetUpPipelineCache()
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VkCR(vkCreatePipelineCache(vbr->device, &pipelineCacheCreateInfo, nullptr, &vbr->pipelineCache));
    }

    void VulkanBase::SetUpCommands()
    {
        /* RENDERING COMMANDS */
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = vbr->graphicsFamily.value();
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandBufferAllocateInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = 1;

        vbc->frames.resize(vbs->frameOverlap);

        for (int i = 0; i < vbs->frameOverlap; i++)
        {
            VkCR(vkCreateCommandPool(vbr->device, &commandPoolInfo, nullptr, &vbc->frames[i].commandPool));

            commandBufferInfo.commandPool = vbc->frames[i].commandPool;
            // default command buffer that we will use for rendering
            VkCR(vkAllocateCommandBuffers(vbr->device, &commandBufferInfo, &vbc->frames[i].commandBuffer));
        }

        /* SINGLE-USE COMMANDS */
        VkCR(vkCreateCommandPool(vbr->device, &commandPoolInfo, nullptr, &vbr->singleUseCommandPool));
    }


    void VulkanBase::SetUpSyncObjects()
    {
        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
        VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
        for (int i = 0; i < vbs->frameOverlap; i++)
        {
            VkCR(vkCreateFence(vbr->device, &fenceInfo, nullptr, &vbc->frames[i].waitFence));
            VkCR(vkCreateSemaphore(vbr->device, &semaphoreInfo, nullptr, &vbc->frames[i].presentSemaphore));
            VkCR(vkCreateSemaphore(vbr->device, &semaphoreInfo, nullptr, &vbc->frames[i].renderSemaphore));
        }
    }

    void VulkanBase::SetUpImGui()
    {
        VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
        poolInfo.pPoolSizes = poolSizes;

        VkCR(vkCreateDescriptorPool(vbr->device, &poolInfo, nullptr, &vbr->imguiDescriptorPool));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        Ref<GLFWwindow> window = Application::Get().GetWindow().GetNativeWindow();
        ImGui_ImplGlfw_InitForVulkan(window.get(), true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = vbr->instance;
        initInfo.Device = vbr->device;
        initInfo.PhysicalDevice = vbr->physicalDevice;
        initInfo.DescriptorPool = vbr->imguiDescriptorPool;
        initInfo.PipelineCache = vbr->pipelineCache;
        initInfo.QueueFamily = vbr->graphicsFamily.value();
        initInfo.Queue = vbr->graphicsQueue;
        initInfo.ImageCount = vbs->frameOverlap;
        initInfo.MinImageCount = 2;
        initInfo.MSAASamples = vbs->multiSampling ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = VK_NULL_HANDLE;

        initInfo.UseDynamicRendering = true;
        // dynamic rendering parameters for imgui to use
        initInfo.PipelineRenderingCreateInfo = {};
        initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vbr->swapchain.colorTarget.format;

        ImGui_ImplVulkan_Init(&initInfo);

        // viewport
        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        vkCreateSampler(vbr->device, &sampler, nullptr, &vbr->viewport.sampler);

        vbr->viewport.images.resize(vbr->swapchain.imageCount);
        vbr->viewport.textureIDs.resize(vbr->swapchain.imageCount);
        for (uint32_t i = 0; i < vbr->viewport.images.size(); i++)
        {
            vbr->viewport.format = VK_FORMAT_B8G8R8A8_SRGB;
            VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

            VkImageUsageFlags usages{};
            usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            usages |= VK_IMAGE_USAGE_SAMPLED_BIT;
            usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            vbr->viewport.images[i] =
                VulkanImage::Create({ vbr->swapchain.extent.width, vbr->swapchain.extent.height, 1 }, vbr->viewport.format, samples, usages, 1, VK_IMAGE_TYPE_2D);

            vbr->viewport.textureIDs[i] =
                ImGui_ImplVulkan_AddTexture(vbr->viewport.sampler, vbr->viewport.images[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        ImGuiLayer::SetStyles();
    }

    void VulkanBase::Render(Ref<EditorCamera> camera)
    {
        VulkanScene::UpdateUniforms(camera);

        if (VulkanScene::IsInvalid())
        {
            VulkanScene::Rebuild();
            return;
        }

        Draw();
    }

    void VulkanBase::Draw()
    {
        auto tStart = std::chrono::high_resolution_clock::now();

        FrameData& frame = vbc->frames[vbc->currentFrame];

        VkCR(vkWaitForFences(vbr->device, 1, &frame.waitFence, VK_TRUE, UINT64_MAX));

        uint32_t scImageIndex;
        VkResult result = vkAcquireNextImageKHR(vbr->device, vbr->swapchain.swapchain, UINT64_MAX, frame.presentSemaphore, VK_NULL_HANDLE, &scImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            WindowResize();
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            DK_CORE_ASSERT(result);
        }

        VkImage msColorTarget = vbr->multisampleTarget.color.image;
        VkFormat msColorTargetFormat = vbr->multisampleTarget.color.format;

        VkImage msDepthTarget = vbr->multisampleTarget.depth.image;
        VkFormat msDepthTargetFormat = vbr->multisampleTarget.depth.format;

        VkImage viewportImage = vbr->viewport.images[scImageIndex].image;
        VkFormat viewportFormat = vbr->viewport.format;

        VkImage scColorTarget = vbr->swapchain.colorTarget.image;
        VkFormat scColorTargetFormat = vbr->swapchain.colorTarget.format;

        VkImage scImage = vbr->swapchain.images[scImageIndex];
        VkFormat scFormat = vbr->swapchain.format;

        // after acquiring image (to avoid deadlock), manually reset the fence to the unsignaled state
        vkResetFences(vbr->device, 1, &frame.waitFence);

        VkCR(vkResetCommandBuffer(frame.commandBuffer, 0));

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VkCR(vkBeginCommandBuffer(frame.commandBuffer, &commandBufferBeginInfo));

        VulkanImage::Transition(frame.commandBuffer, msColorTarget, msColorTargetFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VulkanImage::Transition(frame.commandBuffer, msDepthTarget, msDepthTargetFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        VulkanImage::Transition(frame.commandBuffer, viewportImage, viewportFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VulkanScene::Draw(frame.commandBuffer, scImageIndex);

        VulkanImage::Transition(frame.commandBuffer, viewportImage, viewportFormat, 1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VulkanImage::Transition(frame.commandBuffer, scColorTarget, scColorTargetFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        VulkanImage::Transition(frame.commandBuffer, scImage, scFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        DrawImGui(frame.commandBuffer, scImageIndex);

        VulkanImage::Transition(frame.commandBuffer, scImage, scFormat, 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        VkCR(vkEndCommandBuffer(frame.commandBuffer));

        const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkCommandBufferSubmitInfo commandInfo{};
        commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandInfo.pNext = nullptr;
        commandInfo.commandBuffer = frame.commandBuffer;
        commandInfo.deviceMask = 0;

        VkSemaphoreSubmitInfo presentSemaphoreInfo{};
        presentSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        presentSemaphoreInfo.pNext = nullptr;
        presentSemaphoreInfo.semaphore = frame.presentSemaphore;
        presentSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
        presentSemaphoreInfo.deviceIndex = 0;
        presentSemaphoreInfo.value = 1;

        VkSemaphoreSubmitInfo renderSemaphoreInfo{};
        renderSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        renderSemaphoreInfo.pNext = nullptr;
        renderSemaphoreInfo.semaphore = frame.renderSemaphore;
        renderSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        renderSemaphoreInfo.deviceIndex = 0;
        renderSemaphoreInfo.value = 1;

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &presentSemaphoreInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &renderSemaphoreInfo;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandInfo;

        VkCR(vkQueueSubmit2KHR(vbr->graphicsQueue, 1, &submitInfo, frame.waitFence));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &frame.renderSemaphore;
        VkSwapchainKHR swapChains[] = { vbr->swapchain.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &scImageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(vbr->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR /*|| s_State.framebufferResized*/)
        {
            WindowResize();
            return;
        }
        else if (result != VK_SUCCESS)
        {
            DK_CORE_ASSERT(result);
        }

        vbc->currentFrame = (vbc->currentFrame + 1) % vbs->frameOverlap;

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        float frameTimer = (float)tDiff / 1000.0f;
        // vbr->camera.update(frameTimer);
    }

    void VulkanBase::DrawImGui(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = vbr->swapchain.colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        colorAttachment.resolveImageView = vbr->swapchain.views[imageIndex];
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, vbr->swapchain.extent.width, vbr->swapchain.extent.height };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = nullptr;
        renderInfo.pStencilAttachment = nullptr;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        LayerStack& layerStack = Application::Get().GetLayerStack();

        ImGuiLayer::Begin();
        for (Layer* layer : layerStack)
            layer->OnImGuiRender((ImTextureID)vbr->viewport.textureIDs[imageIndex]);
        ImGuiLayer::End(commandBuffer);

        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanBase::WindowResize()
    {
        VulkanBase::Idle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(vbr->device, vbr->imguiDescriptorPool, nullptr);
        vkDestroySampler(vbr->device, vbr->viewport.sampler, nullptr);

        for (auto image : vbr->viewport.images)
            VulkanImage::Destroy(image);

        VulkanImage::Destroy(vbr->swapchain.colorTarget);
        VulkanImage::Destroy(vbr->multisampleTarget.color);
        VulkanImage::Destroy(vbr->multisampleTarget.depth);

        vkDestroyPipelineCache(vbr->device, vbr->pipelineCache, nullptr);

        SetUpSwapchain();

        SetUpImGui();

        ImGuiLayer::SetStyles();
    }

}
