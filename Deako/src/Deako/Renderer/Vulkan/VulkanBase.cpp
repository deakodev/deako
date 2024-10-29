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

    Ref<VulkanBaseResources> VulkanBase::vb = CreateRef<VulkanBaseResources>();
    Ref<VulkanSceneResources> VulkanScene::vs = CreateRef<VulkanSceneResources>();

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
        vkDeviceWaitIdle(vb->device);
    }

    void VulkanBase::Shutdown()
    {
        VulkanBase::Idle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(vb->device, vb->imguiDescriptorPool, nullptr);

        for (auto& frame : vb->frames)
        {
            vkDestroySemaphore(vb->device, frame.renderSemaphore, nullptr);
            vkDestroySemaphore(vb->device, frame.presentSemaphore, nullptr);
            vkDestroyFence(vb->device, frame.waitFence, nullptr);
            vkDestroyCommandPool(vb->device, frame.commandPool, nullptr);
        }

        vkDestroyCommandPool(vb->device, vb->singleUseCommandPool, nullptr);

        vkDestroyPipelineCache(vb->device, vb->pipelineCache, nullptr);

        for (auto imageView : vb->swapchain.views)
            vkDestroyImageView(vb->device, imageView, nullptr);

        VulkanImage::Destroy(vb->swapchain.colorTarget);
        VulkanImage::Destroy(vb->swapchain.depthTarget);

        vkDestroySwapchainKHR(vb->device, vb->swapchain.swapchain, nullptr);

        vkDestroyDevice(vb->device, nullptr);

        vkDestroySurfaceKHR(vb->instance, vb->surface, nullptr);

        if (vb->settings.validationEnabled) VulkanDebug::DestroyDebugUtilsMessengerEXT();

        vkDestroyInstance(vb->instance, nullptr);
    }

    void VulkanBase::CreateInstance()
    {
        #if defined(VK_VALIDATION)
        vb->settings.validationEnabled = true;
        #endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = GetApplication().GetSpecification().name.c_str();
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
        if (vb->settings.validationEnabled)
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
        if (vb->settings.validationEnabled)
        {
            validationLayerNames.push_back("VK_LAYER_KHRONOS_validation");
            instanceInfo.enabledLayerCount = (uint32_t)validationLayerNames.size();
            instanceInfo.ppEnabledLayerNames = validationLayerNames.data();
            // used to debug instance creation without needing instance reference
            VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
            VulkanDebug::PopulateDebugMessengerCreateInfo(debugInfo);
            instanceInfo.pNext = &debugInfo;
        }

        VkCR(vkCreateInstance(&instanceInfo, nullptr, &vb->instance));
    }

    void VulkanBase::SetUpDebugMessenger()
    {
        if (vb->settings.validationEnabled)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
            VulkanDebug::PopulateDebugMessengerCreateInfo(debugInfo);
            VkCR(VulkanDebug::CreateDebugUtilsMessengerEXT(&debugInfo));
        }
    }

    void VulkanBase::SetUpDevice()
    {
        /* CREATE SURFACE */
        Ref<GLFWwindow> window = GetApplication().GetWindow().GetNativeWindow();
        VkCR(glfwCreateWindowSurface(vb->instance, window.get(), nullptr, &vb->surface));

        /* DETERMINE PHYSICAL */
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vb->instance, &deviceCount, nullptr);
        DK_CORE_ASSERT(deviceCount);  // check if any devices are supported

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(vb->instance, &deviceCount, physicalDevices.data());

        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& physicalDevice : physicalDevices)
        {   // determine scores of each device
            int score = VulkanDevice::RatePhysical(physicalDevice);
            candidates.insert(std::make_pair(score, physicalDevice));
        }

        // determine best candidate and set as physical device
        if (candidates.rbegin()->first > 0) vb->physicalDevice = candidates.rbegin()->second;
        else DK_CORE_ASSERT(false);

        // set family indices of our selected physical device
        VulkanDevice::FindQueueFamilies(vb->physicalDevice);

        /* CREATE LOGICAL */
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // obtain
        std::set<uint32_t> queueFamilies = { vb->graphicsFamily.value(), vb->presentFamily.value() };

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

        VulkanDevice::CheckExtensionSupport(vb->physicalDevice, enabledExtensions);

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

        VkCR(vkCreateDevice(vb->physicalDevice, &deviceInfo, nullptr, &vb->device));

        /* GET QUEUE FAMILIES */
        vkGetDeviceQueue(vb->device, vb->graphicsFamily.value(), 0, &vb->graphicsQueue);
        vkGetDeviceQueue(vb->device, vb->presentFamily.value(), 0, &vb->presentQueue);

        // find certain extension functions since macos doesn't support 1.3
        vkCmdPipelineBarrier2KHR =
            (PFN_vkCmdPipelineBarrier2KHR)vkGetDeviceProcAddr(vb->device, "vkCmdPipelineBarrier2KHR");
        vkQueueSubmit2KHR =
            (PFN_vkQueueSubmit2KHR)vkGetDeviceProcAddr(vb->device, "vkQueueSubmit2KHR");
        if (!vkCmdPipelineBarrier2KHR || !vkQueueSubmit2KHR)
            throw std::runtime_error("Failed to load VK_KHR_synchronization2 functions!");

        vkCmdBeginRenderingKHR =
            (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(vb->device, "vkCmdBeginRenderingKHR");
        vkCmdEndRenderingKHR =
            (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(vb->device, "vkCmdEndRenderingKHR");
        if (!vkCmdBeginRenderingKHR || !vkCmdEndRenderingKHR)
            throw std::runtime_error("Failed to load VK_KHR_dynamic_rendering functions!");
    }

    void VulkanBase::SetUpSwapchain()
    {
        /* SWAPCHAIN CREATION */
        VkSwapchainKHR oldSwapchain = vb->swapchain.swapchain;

        VulkanSwapchain::QuerySupport(vb->physicalDevice);

        VkSurfaceCapabilitiesKHR caps = vb->swapchain.details.capabilities;
        std::vector<VkSurfaceFormatKHR> formats = vb->swapchain.details.formats;
        std::vector<VkPresentModeKHR> presentModes = vb->swapchain.details.presentModes;

        VkSurfaceFormatKHR surfaceFormat = VulkanSwapchain::ChooseSurfaceFormat(formats);
        vb->swapchain.format = surfaceFormat.format;

        VkPresentModeKHR presentMode = VulkanSwapchain::ChoosePresentMode(presentModes);
        vb->swapchain.extent = VulkanSwapchain::ChooseExtent(caps);

        // check to make sure we dont exceed the max number of possible images
        vb->swapchain.imageCount = caps.minImageCount;
        if (caps.maxImageCount > 0 && vb->swapchain.imageCount > caps.maxImageCount)
            vb->swapchain.imageCount = caps.maxImageCount;

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
        swapchainInfo.surface = vb->surface;
        swapchainInfo.minImageCount = vb->swapchain.imageCount;
        swapchainInfo.imageFormat = vb->swapchain.format;
        swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainInfo.imageExtent = vb->swapchain.extent;
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

        uint32_t queueFamilyIndices[] = { vb->graphicsFamily.value(), vb->presentFamily.value() };
        if (vb->graphicsFamily != vb->presentFamily)
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

        VkCR(vkCreateSwapchainKHR(vb->device, &swapchainInfo, nullptr, &vb->swapchain.swapchain));

        // if an existing swap chain is re-created, destroy the old swap chain
        if (oldSwapchain != VK_NULL_HANDLE)
        {
            for (auto imageView : vb->swapchain.views)
                vkDestroyImageView(vb->device, imageView, nullptr);
            vkDestroySwapchainKHR(vb->device, oldSwapchain, nullptr);

            VulkanImage::Destroy(vb->swapchain.colorTarget);
            VulkanImage::Destroy(vb->swapchain.depthTarget);
        }

        /* SWAPCHAIN IMAGES */
        vkGetSwapchainImagesKHR(vb->device, vb->swapchain.swapchain, &vb->swapchain.imageCount, nullptr);
        vb->swapchain.images.resize(vb->swapchain.imageCount);
        vkGetSwapchainImagesKHR(vb->device, vb->swapchain.swapchain, &vb->swapchain.imageCount, vb->swapchain.images.data());

        /* SWAPCHAIN IMAGE VIEWS */
        vb->swapchain.views.resize(vb->swapchain.imageCount);
        for (size_t i = 0; i < vb->swapchain.imageCount; i++)
        {
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.image = vb->swapchain.images[i];
            imageViewInfo.format = vb->swapchain.format;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

            VkCR(vkCreateImageView(vb->device, &imageViewInfo, nullptr, &vb->swapchain.views[i]));
        }

        /* DRAW TARGETS */ // resolve to the sc image view
        VkExtent3D targetExtent = { vb->swapchain.extent.width, vb->swapchain.extent.height, 1 };
        VkSampleCountFlagBits targetSamples = vb->settings.sampleCount;

        VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
        VkImageUsageFlags colorUsages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        vb->swapchain.colorTarget =
            VulkanImage::Create(targetExtent, colorFormat, targetSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        VkImageUsageFlags depthUsages = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        vb->swapchain.depthTarget =
            VulkanImage::Create(targetExtent, depthFormat, targetSamples, depthUsages, 1, VK_IMAGE_TYPE_2D);
    }

    void VulkanBase::SetUpPipelineCache()
    {
        if (vb->pipelineCache) vkDestroyPipelineCache(vb->device, vb->pipelineCache, nullptr);

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VkCR(vkCreatePipelineCache(vb->device, &pipelineCacheCreateInfo, nullptr, &vb->pipelineCache));
    }

    void VulkanBase::SetUpCommands()
    {
        /* RENDERING COMMANDS */
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = vb->graphicsFamily.value();
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandBufferAllocateInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = 1;

        vb->frames.resize(vb->settings.frameOverlap);

        for (int i = 0; i < vb->settings.frameOverlap; i++)
        {
            VkCR(vkCreateCommandPool(vb->device, &commandPoolInfo, nullptr, &vb->frames[i].commandPool));

            commandBufferInfo.commandPool = vb->frames[i].commandPool;
            // default command buffer that we will use for rendering
            VkCR(vkAllocateCommandBuffers(vb->device, &commandBufferInfo, &vb->frames[i].commandBuffer));
        }

        /* SINGLE-USE COMMANDS */
        VkCR(vkCreateCommandPool(vb->device, &commandPoolInfo, nullptr, &vb->singleUseCommandPool));
    }


    void VulkanBase::SetUpSyncObjects()
    {
        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
        VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
        for (int i = 0; i < vb->settings.frameOverlap; i++)
        {
            VkCR(vkCreateFence(vb->device, &fenceInfo, nullptr, &vb->frames[i].waitFence));
            VkCR(vkCreateSemaphore(vb->device, &semaphoreInfo, nullptr, &vb->frames[i].presentSemaphore));
            VkCR(vkCreateSemaphore(vb->device, &semaphoreInfo, nullptr, &vb->frames[i].renderSemaphore));
        }
    }

    void VulkanBase::SetUpImGui()
    {
        VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }, };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
        poolInfo.pPoolSizes = poolSizes;

        VkCR(vkCreateDescriptorPool(vb->device, &poolInfo, nullptr, &vb->imguiDescriptorPool));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

        Ref<GLFWwindow> window = GetApplication().GetWindow().GetNativeWindow();
        ImGui_ImplGlfw_InitForVulkan(window.get(), true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = vb->instance;
        initInfo.Device = vb->device;
        initInfo.PhysicalDevice = vb->physicalDevice;
        initInfo.DescriptorPool = vb->imguiDescriptorPool;
        initInfo.PipelineCache = vb->pipelineCache;
        initInfo.QueueFamily = vb->graphicsFamily.value();
        initInfo.Queue = vb->graphicsQueue;
        initInfo.ImageCount = vb->settings.frameOverlap;
        initInfo.MinImageCount = 2;
        initInfo.MSAASamples = vb->settings.sampleCount;
        initInfo.Allocator = VK_NULL_HANDLE;

        initInfo.UseDynamicRendering = true;
        // dynamic rendering parameters for imgui to use
        initInfo.PipelineRenderingCreateInfo = {};
        initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vb->swapchain.colorTarget.format;

        ImGui_ImplVulkan_Init(&initInfo);

        ImGuiLayer::SetStyles();
    }

    void VulkanBase::Render()
    {
        VulkanScene::UpdateUniforms();

        if (VulkanScene::IsInvalid())
        {
            VulkanScene::Rebuild(); return;
        }

        Draw();
    }

    void VulkanBase::Draw()
    {
        FrameData& frame = vb->frames[vb->context.currentFrame];

        VkCR(vkWaitForFences(vb->device, 1, &frame.waitFence, VK_TRUE, UINT64_MAX));

        uint32_t scImageIndex;
        VkResult result = vkAcquireNextImageKHR(vb->device, vb->swapchain.swapchain, UINT64_MAX, frame.presentSemaphore, VK_NULL_HANDLE, &scImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapchain(); return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            DK_CORE_ASSERT(result);
        }

        // manually reset fence after acquiring image (to avoid deadlock)
        vkResetFences(vb->device, 1, &frame.waitFence);

        VkCR(vkResetCommandBuffer(frame.commandBuffer, 0));

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VkCR(vkBeginCommandBuffer(frame.commandBuffer, &commandBufferBeginInfo));

        VulkanImage::Transition(frame.commandBuffer, vb->swapchain.colorTarget.image, vb->swapchain.colorTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        VulkanScene::DrawPicking(frame.commandBuffer, scImageIndex);

        VulkanImage::Transition(frame.commandBuffer, vb->swapchain.depthTarget.image, vb->swapchain.depthTarget.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        VulkanScene::Draw(frame.commandBuffer, scImageIndex);

        VulkanImage::Transition(frame.commandBuffer, vb->swapchain.images[scImageIndex], vb->swapchain.format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        DrawGui(frame.commandBuffer, scImageIndex);

        VulkanImage::Transition(frame.commandBuffer, vb->swapchain.images[scImageIndex], vb->swapchain.format, 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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

        VkCR(vkQueueSubmit2KHR(vb->graphicsQueue, 1, &submitInfo, frame.waitFence));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &frame.renderSemaphore;
        VkSwapchainKHR swapChains[] = { vb->swapchain.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &scImageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(vb->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            RecreateSwapchain(); return;
        }
        else if (result != VK_SUCCESS)
        {
            DK_CORE_ASSERT(result);
        }

        vb->context.currentFrame = (vb->context.currentFrame + 1) % vb->settings.frameOverlap;
    }

    void VulkanBase::DrawGui(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = vb->swapchain.colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        colorAttachment.resolveImageView = vb->swapchain.views[imageIndex];
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, vb->swapchain.extent.width, vb->swapchain.extent.height };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = nullptr;
        renderInfo.pStencilAttachment = nullptr;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        LayerStack& layerStack = GetApplication().GetLayerStack();

        ImGuiLayer::Begin();
        for (Layer* layer : layerStack)
            layer->OnImGuiRender();
        ImGuiLayer::End(commandBuffer);

        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanBase::RecreateSwapchain()
    {
        VulkanBase::Idle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(vb->device, vb->imguiDescriptorPool, nullptr);

        SetUpSwapchain();

        SetUpPipelineCache();

        SetUpImGui();
    }

}
