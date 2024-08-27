#include "VulkanBase.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"
#include "Deako/Asset/AssetPoolWrapper.h"
#include "Deako/Asset/GLTFImporter.h"
#include "Deako/Project/Project.h"
#include "Deako/Scene/Scene.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Deako {

    Ref<VulkanResources> VulkanBase::vr = CreateRef<VulkanResources>();
    Ref<VulkanSettings> VulkanBase::vs = CreateRef<VulkanSettings>();

    PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR = nullptr;
    PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR = nullptr;
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = nullptr;
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = nullptr;

    void VulkanBase::Init(const char* appName)
    {
        CreateInstance(appName);

        SetUpDebugMessenger();

        SetUpDevice();

        SetUpSwapchain();

        SetUpCommands();

        SetUpSyncObjects();

        SetUpAssets();

        vr->camera.type = Camera::CameraType::lookat;
        vr->camera.setPerspective(45.0f, (float)vr->swapchain.extent.width / (float)vr->swapchain.extent.height, 0.01f, 256.0f);
        vr->camera.rotationSpeed = 0.25f;
        vr->camera.movementSpeed = 0.1f;
        vr->camera.setPosition({ 0.0f, 0.0f, 10.0f });
        vr->camera.setRotation({ 0.0f, 0.0f, 0.0f });
        vr->camera.updateViewMatrix();

        SetUpUniforms();

        SetUpDescriptors();

        SetUpPipelines();

        SetUpImGui();

        vr->prepared = true;
    }

    void VulkanBase::Idle()
    {
        vkDeviceWaitIdle(vr->device);
    }

    void VulkanBase::Shutdown()
    {
        VulkanBase::Idle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(vr->device, vr->imguiDescriptorPool, nullptr);
        vkDestroySampler(vr->device, vr->viewport.sampler, nullptr);

        for (auto image : vr->viewport.images)
            VulkanImage::Destroy(image);

        for (auto& [prefix, pipeline] : vr->pipelines)
            vkDestroyPipeline(vr->device, pipeline, nullptr);
        vkDestroyPipelineLayout(vr->device, vr->pipelineLayout, nullptr);
        vkDestroyPipelineCache(vr->device, vr->pipelineCache, nullptr);

        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.scene, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.material, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.node, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, vr->descriptorSetLayouts.materialBuffer, nullptr);
        vkDestroyDescriptorPool(vr->device, vr->descriptorPool, nullptr);

        for (auto& uniform : vr->uniforms)
        {
            VulkanBuffer::Destroy(uniform.dynamic.buffer);
            VulkanBuffer::Destroy(uniform.shared.buffer);
            VulkanBuffer::Destroy(uniform.params.buffer);
        }

        VulkanBuffer::Destroy(vr->shaderMaterialBuffer);

        for (auto& [tag, model] : vr->propModels)
        {
            if (model) AssetPool::DestroyAsset<Model>(model->m_Handle);
        }
        for (auto& [tag, model] : vr->environmentModels)
        {
            if (model) AssetPool::DestroyAsset<Model>(model->m_Handle);
        }

        vr->textures.lutBrdf->Destroy();
        vr->textures.irradianceCube->Destroy();
        vr->textures.prefilteredCube->Destroy();
        vr->textures.environmentCube->Destroy();
        vr->textures.empty->Destroy();

        for (auto& frame : vr->frames)
        {
            vkDestroySemaphore(vr->device, frame.renderSemaphore, nullptr);
            vkDestroySemaphore(vr->device, frame.presentSemaphore, nullptr);
            vkDestroyFence(vr->device, frame.waitFence, nullptr);
            vkDestroyCommandPool(vr->device, frame.commandPool, nullptr);
        }

        vkDestroyCommandPool(vr->device, vr->singleUseCommandPool, nullptr);

        for (auto imageView : vr->swapchain.views)
            vkDestroyImageView(vr->device, imageView, nullptr);

        VulkanImage::Destroy(vr->swapchain.colorTarget);
        VulkanImage::Destroy(vr->multisampleTarget.color);
        VulkanImage::Destroy(vr->multisampleTarget.depth);

        vkDestroySwapchainKHR(vr->device, vr->swapchain.swapchain, nullptr);

        vkDestroyDevice(vr->device, nullptr);

        vkDestroySurfaceKHR(vr->instance, vr->surface, nullptr);

        if (vs->validationEnabled) VulkanDebug::DestroyDebugUtilsMessengerEXT();

        vkDestroyInstance(vr->instance, nullptr);
    }

    void VulkanBase::CreateInstance(const char* appName)
    {
        #if defined(VK_VALIDATION)
        vs->validationEnabled = true;
        #endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName;
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
        if (vs->validationEnabled)
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
        if (vs->validationEnabled)
        {
            validationLayerNames.push_back("VK_LAYER_KHRONOS_validation");
            instanceInfo.enabledLayerCount = (uint32_t)validationLayerNames.size();
            instanceInfo.ppEnabledLayerNames = validationLayerNames.data();
            // used to debug instance creation without needing instance reference
            VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
            VulkanDebug::PopulateDebugMessengerCreateInfo(debugInfo);
            instanceInfo.pNext = &debugInfo;
        }

        VkCR(vkCreateInstance(&instanceInfo, nullptr, &vr->instance));
    }

    void VulkanBase::SetUpDebugMessenger()
    {
        if (vs->validationEnabled)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
            VulkanDebug::PopulateDebugMessengerCreateInfo(debugInfo);
            VkCR(VulkanDebug::CreateDebugUtilsMessengerEXT(&debugInfo));
        }
    }

    void VulkanBase::SetUpDevice()
    {
        /* CREATE SURFACE */
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        VkCR(glfwCreateWindowSurface(vr->instance, window, nullptr, &vr->surface));

        /* DETERMINE PHYSICAL */
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vr->instance, &deviceCount, nullptr);
        DK_CORE_ASSERT(deviceCount);  // check if any devices are supported

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(vr->instance, &deviceCount, physicalDevices.data());

        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& physicalDevice : physicalDevices)
        {   // determine scores of each device
            int score = VulkanDevice::RatePhysical(physicalDevice);
            candidates.insert(std::make_pair(score, physicalDevice));
        }

        // determine best candidate and set as physical device
        if (candidates.rbegin()->first > 0) vr->physicalDevice = candidates.rbegin()->second;
        else DK_CORE_ASSERT(false);

        // set family indices of our selected physical device
        VulkanDevice::FindQueueFamilies(vr->physicalDevice);

        /* CREATE LOGICAL */
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // obtain
        std::set<uint32_t> queueFamilies = { vr->graphicsFamily.value(), vr->presentFamily.value() };

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

        VulkanDevice::CheckExtensionSupport(vr->physicalDevice, enabledExtensions);

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

        VkCR(vkCreateDevice(vr->physicalDevice, &deviceInfo, nullptr, &vr->device));

        /* GET QUEUE FAMILIES */
        vkGetDeviceQueue(vr->device, vr->graphicsFamily.value(), 0, &vr->graphicsQueue);
        vkGetDeviceQueue(vr->device, vr->presentFamily.value(), 0, &vr->presentQueue);

        // find certain extension functions since macos doesn't support 1.3
        vkCmdPipelineBarrier2KHR =
            (PFN_vkCmdPipelineBarrier2KHR)vkGetDeviceProcAddr(vr->device, "vkCmdPipelineBarrier2KHR");
        vkQueueSubmit2KHR =
            (PFN_vkQueueSubmit2KHR)vkGetDeviceProcAddr(vr->device, "vkQueueSubmit2KHR");
        if (!vkCmdPipelineBarrier2KHR || !vkQueueSubmit2KHR)
            throw std::runtime_error("Failed to load VK_KHR_synchronization2 functions!");

        vkCmdBeginRenderingKHR =
            (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(vr->device, "vkCmdBeginRenderingKHR");
        vkCmdEndRenderingKHR =
            (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(vr->device, "vkCmdEndRenderingKHR");
        if (!vkCmdBeginRenderingKHR || !vkCmdEndRenderingKHR)
            throw std::runtime_error("Failed to load VK_KHR_dynamic_rendering functions!");
    }

    void VulkanBase::SetUpSwapchain()
    {
        /* SWAPCHAIN CREATION */
        VkSwapchainKHR oldSwapchain = vr->swapchain.swapchain;

        VulkanSwapchain::QuerySupport(vr->physicalDevice);

        VkSurfaceCapabilitiesKHR caps = vr->swapchain.details.capabilities;
        std::vector<VkSurfaceFormatKHR> formats = vr->swapchain.details.formats;
        std::vector<VkPresentModeKHR> presentModes = vr->swapchain.details.presentModes;

        VkSurfaceFormatKHR surfaceFormat = VulkanSwapchain::ChooseSurfaceFormat(formats);
        vr->swapchain.format = surfaceFormat.format;

        VkPresentModeKHR presentMode = VulkanSwapchain::ChoosePresentMode(presentModes);
        vr->swapchain.extent = VulkanSwapchain::ChooseExtent(caps);

        // check to make sure we dont exceed the max number of possible images
        vr->swapchain.imageCount = caps.minImageCount;
        if (caps.maxImageCount > 0 && vr->swapchain.imageCount > caps.maxImageCount)
            vr->swapchain.imageCount = caps.maxImageCount;

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
        swapchainInfo.surface = vr->surface;
        swapchainInfo.minImageCount = vr->swapchain.imageCount;
        swapchainInfo.imageFormat = vr->swapchain.format;
        swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainInfo.imageExtent = vr->swapchain.extent;
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

        uint32_t queueFamilyIndices[] = { vr->graphicsFamily.value(), vr->presentFamily.value() };
        if (vr->graphicsFamily != vr->presentFamily)
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

        VkCR(vkCreateSwapchainKHR(vr->device, &swapchainInfo, nullptr, &vr->swapchain.swapchain));

        // if an existing swap chain is re-created, destroy the old swap chain
        if (oldSwapchain != VK_NULL_HANDLE)
        {
            for (auto imageView : vr->swapchain.views)
                vkDestroyImageView(vr->device, imageView, nullptr);
            vkDestroySwapchainKHR(vr->device, oldSwapchain, nullptr);
        }

        /* SWAPCHAIN IMAGES */
        vkGetSwapchainImagesKHR(vr->device, vr->swapchain.swapchain, &vr->swapchain.imageCount, nullptr);
        vr->swapchain.images.resize(vr->swapchain.imageCount);
        vkGetSwapchainImagesKHR(vr->device, vr->swapchain.swapchain, &vr->swapchain.imageCount, vr->swapchain.images.data());

        /* SWAPCHAIN IMAGE VIEWS */
        vr->swapchain.views.resize(vr->swapchain.imageCount);
        for (size_t i = 0; i < vr->swapchain.imageCount; i++)
        {
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.image = vr->swapchain.images[i];
            imageViewInfo.format = vr->swapchain.format;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

            VkCR(vkCreateImageView(vr->device, &imageViewInfo, nullptr, &vr->swapchain.views[i]));
        }

        /* COLOR TARGET */ // resolves to the sc image view
        VkExtent3D colorExtent = { vr->swapchain.extent.width, vr->swapchain.extent.height, 1 };
        VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
        VkSampleCountFlagBits colorSamples = VK_SAMPLE_COUNT_4_BIT;

        VkImageUsageFlags colorUsages{};
        colorUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        colorUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vr->swapchain.colorTarget =
            VulkanImage::Create(colorExtent, colorFormat, colorSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        /* MULTISAMPLE TARGET */ // resolves to the viewport image view
        vr->multisampleTarget.color =
            VulkanImage::Create(colorExtent, colorFormat, colorSamples, colorUsages, 1, VK_IMAGE_TYPE_2D);

        VkExtent3D depthExtent = { vr->swapchain.extent.width, vr->swapchain.extent.height, 1 };
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        VkSampleCountFlagBits depthSamples = VK_SAMPLE_COUNT_4_BIT;

        VkImageUsageFlags depthUsages{};
        depthUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vr->multisampleTarget.depth =
            VulkanImage::Create(depthExtent, depthFormat, depthSamples, depthUsages, 1, VK_IMAGE_TYPE_2D);

        /* PIPELINE CACHE */
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VkCR(vkCreatePipelineCache(vr->device, &pipelineCacheCreateInfo, nullptr, &vr->pipelineCache));
    }

    void VulkanBase::SetUpCommands()
    {
        /* RENDERING COMMANDS */
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = vr->graphicsFamily.value();
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandBufferAllocateInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = 1;

        vr->frames.resize(vs->frameOverlap);

        for (int i = 0; i < vs->frameOverlap; i++)
        {
            VkCR(vkCreateCommandPool(vr->device, &commandPoolInfo, nullptr, &vr->frames[i].commandPool));

            commandBufferInfo.commandPool = vr->frames[i].commandPool;
            // default command buffer that we will use for rendering
            VkCR(vkAllocateCommandBuffers(vr->device, &commandBufferInfo, &vr->frames[i].commandBuffer));
        }

        /* SINGLE-USE COMMANDS */
        VkCR(vkCreateCommandPool(vr->device, &commandPoolInfo, nullptr, &vr->singleUseCommandPool));
    }


    void VulkanBase::SetUpSyncObjects()
    {
        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
        VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
        for (int i = 0; i < vs->frameOverlap; i++)
        {
            VkCR(vkCreateFence(vr->device, &fenceInfo, nullptr, &vr->frames[i].waitFence));
            VkCR(vkCreateSemaphore(vr->device, &semaphoreInfo, nullptr, &vr->frames[i].presentSemaphore));
            VkCR(vkCreateSemaphore(vr->device, &semaphoreInfo, nullptr, &vr->frames[i].renderSemaphore));
        }
    }

    void VulkanBase::SetUpAssets()
    {
        Ref<Project> project = Project::Open("sandbox/sandbox.proj.deako");

        vr->textures.empty = AssetPool::ImportAsset<Texture2D>("textures/empty.ktx");

        Ref<Scene> scene = AssetPool::ImportAsset<Scene>("scenes/sandbox.scene.deako");

        Scene::SetActiveScene(scene);

        const Scene::Registry& registry = scene->GetRegistry();

        Ref<Model> helmet = AssetPool::ImportGLTF("models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf");
        Ref<Model> box = AssetPool::ImportGLTF("models/Box/glTF-Embedded/Box.gltf");

        auto modalEntities = registry.view<TagComponent, ModelComponent>();
        for (auto& entity : modalEntities)
        {
            auto [tagComp, modelComp] = modalEntities.get<TagComponent, ModelComponent>(entity);

            if (modelComp.usage == ModelComponent::Usage::PROP)
            {
                vr->propModels[tagComp.tag] = helmet;
            }
            else if (modelComp.usage == ModelComponent::Usage::ENVIRONMENT)
            {
                vr->environmentModels[tagComp.tag] = box;
            }
            else if (modelComp.usage == ModelComponent::Usage::NONE)
            {
                DK_CORE_WARN("{0} model not assigned usage!", tagComp.tag);
            }
        }

        CreateMaterialBuffer();

        vr->textures.environmentCube = AssetPool::ImportAsset<TextureCubeMap>("environments/papermill.ktx");
        vr->textures.irradianceCube->GenerateCubeMap();
        vr->textures.prefilteredCube->GenerateCubeMap();

        GenerateBRDFLookUpTable();
    }

    void VulkanBase::SetUpUniforms()
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vr->physicalDevice, &deviceProperties);

        // determine required alignment based on min device offset alignment
        size_t minUniformAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        vr->dynamicUniformAlignment = sizeof(glm::mat4);

        if (minUniformAlignment > 0)
            vr->dynamicUniformAlignment = (vr->dynamicUniformAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

        size_t dynamicBufferSize = vr->propModels.size() * vr->dynamicUniformAlignment;

        vr->uniformDataDynamic.model = (glm::mat4*)VulkanMemory::AlignedAlloc(dynamicBufferSize, vr->dynamicUniformAlignment);
        DK_CORE_ASSERT(vr->uniformDataDynamic.model);

        VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        vr->uniforms.resize(vr->swapchain.imageCount);
        for (auto& uniform : vr->uniforms)
        {
            // dynamic uniform buffer object with model matrix
            uniform.dynamic.buffer = VulkanBuffer::Create(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniform.dynamic.descriptor = { uniform.dynamic.buffer.buffer, 0, vr->dynamicUniformAlignment };
            VkCR(vkMapMemory(vr->device, uniform.dynamic.buffer.memory, 0, dynamicBufferSize, 0, &uniform.dynamic.buffer.mapped));

            // shared uniform buffer object with projection and view matrix
            uniform.shared.buffer = VulkanBuffer::Create(sizeof(vr->uniformDataShared), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.shared.descriptor = { uniform.shared.buffer.buffer, 0, sizeof(vr->uniformDataShared) };
            VkCR(vkMapMemory(vr->device, uniform.shared.buffer.memory, 0, sizeof(vr->uniformDataShared), 0, &uniform.shared.buffer.mapped));

            // shared uniform buffer object with light params
            uniform.params.buffer = VulkanBuffer::Create(sizeof(vr->shaderValuesParams), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryFlags);
            uniform.params.descriptor = { uniform.params.buffer.buffer, 0, sizeof(vr->shaderValuesParams) };
            VkCR(vkMapMemory(vr->device, uniform.params.buffer.memory, 0, sizeof(vr->shaderValuesParams), 0, &uniform.params.buffer.mapped));
        }

        UpdateUniforms();
    }

    void VulkanBase::UpdateUniforms()
    {
        // shared scene
        vr->uniformDataShared.projection = vr->camera.matrices.perspective;
        vr->uniformDataShared.view = vr->camera.matrices.view;

        glm::mat4 cv = glm::inverse(vr->camera.matrices.view);
        vr->uniformDataShared.camPos = glm::vec3(cv[3]);

        // models
        uint32_t index = 0;
        for (auto& [tag, model] : vr->propModels)
        {
            // aligned offset
            glm::mat4* modelMatrix = (glm::mat4*)(((uint64_t)vr->uniformDataDynamic.model + (index * vr->dynamicUniformAlignment)));

            glm::mat4 aabb = vr->propModels[tag]->aaBoundingBox;

            float scaleX = glm::length(glm::vec3(aabb[0]));
            float scaleY = glm::length(glm::vec3(aabb[1]));
            float scaleZ = glm::length(glm::vec3(aabb[2]));
            float scaleFactor = (1.0f / std::max(scaleX, std::max(scaleY, scaleZ))) * 2.0f;

            glm::vec3 aabbMin = glm::vec3(aabb[3][0], aabb[3][1], aabb[3][2]);
            glm::vec3 aabbMax = aabbMin + glm::vec3(scaleX, scaleY, scaleZ);
            glm::vec3 centroid = (aabbMin + aabbMax) / 2.0f; // center of the models aabb
            glm::vec3 translate = -centroid;

            glm::vec3 sideBySideTranslate = glm::vec3((index * 2.0f), 0.0f, 0.0f);

            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            // construct model matrix
            *modelMatrix = glm::translate(glm::mat4(1.0f), translate + sideBySideTranslate) *
                glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));

            index++;
        }

        VulkanResources::UniformSet uniformSet = vr->uniforms[vr->currentFrame];
        memcpy(uniformSet.dynamic.buffer.mapped, vr->uniformDataDynamic.model, vr->dynamicUniformAlignment * vr->propModels.size());
        memcpy(uniformSet.shared.buffer.mapped, &vr->uniformDataShared, sizeof(vr->uniformDataShared));
        memcpy(uniformSet.params.buffer.mapped, &vr->shaderValuesParams, sizeof(vr->shaderValuesParams));

        // flush dynamic uniform to make changes visible to the host
        VkMappedMemoryRange memoryRange{};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = uniformSet.dynamic.buffer.memory;
        memoryRange.size = vr->dynamicUniformAlignment * vr->propModels.size();
        vkFlushMappedMemoryRanges(vr->device, 1, &memoryRange);
    }

    void VulkanBase::SetUpDescriptors()
    {
        /* DESCRIPTOR POOL */
        uint32_t imageSamplerCount = 0;
        uint32_t materialCount = 0;
        uint32_t meshCount = 0;

        // environment samplers (radiance, irradiance, brdf lut)
        imageSamplerCount += 3;

        for (auto& [tag, model] : vr->propModels)
        {
            for (auto& material : model->materials)
            {
                imageSamplerCount += 5;
                materialCount++;
            }

            for (auto node : model->linearNodes)
                if (node->mesh) meshCount++;
        }

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * vr->swapchain.imageCount },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, (4 + meshCount) * vr->swapchain.imageCount },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * vr->swapchain.imageCount },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 } // One SSBO for the shader material buffer
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = (3 + materialCount + meshCount) * vr->swapchain.imageCount;
        VkCR(vkCreateDescriptorPool(vr->device, &poolInfo, nullptr, &vr->descriptorPool));

        /* DESCRIPTOR SETS */
        vr->descriptorSets.resize(vr->swapchain.imageCount);

        {   // scene (matrices and environment maps)
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
            };

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pBindings = setLayoutBindings.data();
            layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
            VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.scene));

            for (auto i = 0; i < vr->descriptorSets.size(); i++)
            {
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = vr->descriptorPool;
                allocInfo.pSetLayouts = &vr->descriptorSetLayouts.scene;
                allocInfo.descriptorSetCount = 1;
                VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &vr->descriptorSets[i].scene));

                std::array<VkWriteDescriptorSet, 6> write{};

                write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[0].descriptorCount = 1;
                write[0].dstSet = vr->descriptorSets[i].scene;
                write[0].dstBinding = 0;
                write[0].pBufferInfo = &vr->uniforms[i].shared.descriptor;

                write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[1].descriptorCount = 1;
                write[1].dstSet = vr->descriptorSets[i].scene;
                write[1].dstBinding = 1;
                write[1].pBufferInfo = &vr->uniforms[i].params.descriptor;

                write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[2].descriptorCount = 1;
                write[2].dstSet = vr->descriptorSets[i].scene;
                write[2].dstBinding = 2;
                write[2].pImageInfo = &vr->textures.irradianceCube->descriptor;

                write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[3].descriptorCount = 1;
                write[3].dstSet = vr->descriptorSets[i].scene;
                write[3].dstBinding = 3;
                write[3].pImageInfo = &vr->textures.prefilteredCube->descriptor;

                write[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[4].descriptorCount = 1;
                write[4].dstSet = vr->descriptorSets[i].scene;
                write[4].dstBinding = 4;
                write[4].pImageInfo = &vr->textures.lutBrdf->descriptor;

                write[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                write[5].descriptorCount = 1;
                write[5].dstSet = vr->descriptorSets[i].scene;
                write[5].dstBinding = 5;
                write[5].pBufferInfo = &vr->uniforms[i].dynamic.descriptor;


                vkUpdateDescriptorSets(vr->device, static_cast<uint32_t>(write.size()), write.data(), 0, NULL);
            }
        }

        {   // material (samplers)
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
            };

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pBindings = setLayoutBindings.data();
            layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
            VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.material));

            // per-material descriptor sets
            for (auto& [tag, model] : vr->propModels)
            {
                for (auto& material : model->materials)
                {
                    VkDescriptorImageInfo emptyTextureDescriptor = vr->textures.empty->descriptor;

                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = vr->descriptorPool;
                    allocInfo.pSetLayouts = &vr->descriptorSetLayouts.material;
                    allocInfo.descriptorSetCount = 1;
                    VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &material->descriptorSet));

                    std::vector<VkDescriptorImageInfo> imageDescriptors = {
                        emptyTextureDescriptor,
                        emptyTextureDescriptor,
                        material->normalTexture ? material->normalTexture->descriptor : emptyTextureDescriptor,
                        material->occlusionTexture ? material->occlusionTexture->descriptor : emptyTextureDescriptor,
                        material->emissiveTexture ? material->emissiveTexture->descriptor : emptyTextureDescriptor
                    };

                    if (material->pbrWorkflows.metallicRoughness)
                    {
                        if (material->baseColorTexture)
                            imageDescriptors[0] = material->baseColorTexture->descriptor;
                        if (material->metallicRoughnessTexture)
                            imageDescriptors[1] = material->metallicRoughnessTexture->descriptor;
                    }
                    else if (material->pbrWorkflows.specularGlossiness)
                    {
                        if (material->extension.diffuseTexture)
                            imageDescriptors[0] = material->extension.diffuseTexture->descriptor;
                        if (material->extension.specularGlossinessTexture)
                            imageDescriptors[1] = material->extension.specularGlossinessTexture->descriptor;
                    }

                    std::array<VkWriteDescriptorSet, 5> write{};
                    for (size_t i = 0; i < imageDescriptors.size(); i++)
                    {
                        write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write[i].descriptorCount = 1;
                        write[i].dstSet = material->descriptorSet;
                        write[i].dstBinding = static_cast<uint32_t>(i);
                        write[i].pImageInfo = &imageDescriptors[i];
                    }

                    vkUpdateDescriptorSets(vr->device, static_cast<uint32_t>(write.size()), write.data(), 0, NULL);
                }
            }

            {   // model node (matrices)
                std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
                };

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.pBindings = setLayoutBindings.data();
                layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
                VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.node));

                for (auto& [tag, model] : vr->propModels)
                {
                    for (auto& node : model->nodes) // per-node descriptor set
                        node->SetDescriptorSet();
                }
            }

            {   // material buffer
                std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                    { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                };
                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.pBindings = setLayoutBindings.data();
                layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
                VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &vr->descriptorSetLayouts.materialBuffer));

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = vr->descriptorPool;
                allocInfo.pSetLayouts = &vr->descriptorSetLayouts.materialBuffer;
                allocInfo.descriptorSetCount = 1;
                VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &vr->shaderMaterialDescriptorSet));

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                write.descriptorCount = 1;
                write.dstSet = vr->shaderMaterialDescriptorSet;
                write.dstBinding = 0;
                write.pBufferInfo = &vr->shaderMaterialDescriptorInfo;
                vkUpdateDescriptorSets(vr->device, 1, &write, 0, nullptr);
            }
        }

        {   // skybox (fixed set)
            for (auto i = 0; i < vr->uniforms.size(); i++)
            {
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = vr->descriptorPool;
                allocInfo.pSetLayouts = &vr->descriptorSetLayouts.scene;
                allocInfo.descriptorSetCount = 1;
                VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &vr->descriptorSets[i].skybox));

                std::array<VkWriteDescriptorSet, 3> write{};
                write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[0].descriptorCount = 1;
                write[0].dstSet = vr->descriptorSets[i].skybox;
                write[0].dstBinding = 0;
                write[0].pBufferInfo = &vr->uniforms[i].shared.descriptor;

                write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[1].descriptorCount = 1;
                write[1].dstSet = vr->descriptorSets[i].skybox;
                write[1].dstBinding = 1;
                write[1].pBufferInfo = &vr->uniforms[i].params.descriptor;

                write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write[2].descriptorCount = 1;
                write[2].dstSet = vr->descriptorSets[i].skybox;
                write[2].dstBinding = 2;
                write[2].pImageInfo = &vr->textures.prefilteredCube->descriptor;

                vkUpdateDescriptorSets(vr->device, static_cast<uint32_t>(write.size()), write.data(), 0, nullptr);
            }
        }
    }

    void VulkanBase::SetUpPipelines()
    {
        // depending on material setting, we need different pipeline variants per set
        // eg. one with back-face culling, one without and one with alpha-blending enabled
        AddPipelineSet("skybox", "shaders/bin/skybox.vert.spv", "shaders/bin/skybox.frag.spv");

        AddPipelineSet("pbr", "shaders/bin/pbr.vert.spv", "shaders/bin/material_pbr.frag.spv");

        AddPipelineSet("unlit", "shaders/bin/pbr.vert.spv", "shaders/bin/material_unlit.frag.spv");
    }

    void VulkanBase::AddPipelineSet(const std::string prefix, std::filesystem::path vertexShader, std::filesystem::path fragmentShader)
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterization{};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &blendAttachment;
        colorBlend.pNext = nullptr;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = (prefix == "skybox" ? VK_FALSE : VK_TRUE);
        depthStencil.depthWriteEnable = (prefix == "skybox" ? VK_FALSE : VK_TRUE);
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.front = depthStencil.back;
        depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

        VkPipelineViewportStateCreateInfo viewport{};
        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport.viewportCount = 1;
        viewport.scissorCount = 1;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        if (vs->multiSampling)
            multisample.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

        std::vector<VkDynamicState> dynamicEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.pDynamicStates = dynamicEnables.data();
        dynamic.dynamicStateCount = static_cast<uint32_t>(dynamicEnables.size());

        std::vector<VkDescriptorSetLayout> setLayouts = {
                vr->descriptorSetLayouts.scene,
                vr->descriptorSetLayouts.material,
                vr->descriptorSetLayouts.node,
                vr->descriptorSetLayouts.materialBuffer,
        };

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.size = sizeof(uint32_t);
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        if (vr->pipelineLayout == VK_NULL_HANDLE)
        {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
            pipelineLayoutInfo.pSetLayouts = setLayouts.data();
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            VkCR(vkCreatePipelineLayout(vr->device, &pipelineLayoutInfo, nullptr, &vr->pipelineLayout));
        }

        // vertex bindings and attributes
        VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

        std::vector<VkVertexInputAttributeDescription>  vertexInputAttributes;

        if (prefix == "skybox")
        {
            vertexInputAttributes = {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos)},
                { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal) },
                { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv0) },
            };
        }
        else
        {
            vertexInputAttributes = {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, pos)},
                { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Model::Vertex, normal) },
                { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv0) },
                { 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Model::Vertex, uv1) },
                { 4, 0, VK_FORMAT_R32G32B32A32_UINT, offsetof(Model::Vertex, joint0) },
                { 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, weight0) },
                { 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Model::Vertex, color) }
            };
        }

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &vertexInputBinding;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInput.pVertexAttributeDescriptions = vertexInputAttributes.data();

        // shaders
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].pName = "main";
        shaderStages[0].module = VulkanShader::CreateShaderModule(vertexShader);

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].pName = "main";
        shaderStages[1].module = VulkanShader::CreateShaderModule(fragmentShader);

        // pipeline rendering info
        VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &vr->multisampleTarget.color.format;
        pipelineRenderingCreateInfo.depthAttachmentFormat = vr->multisampleTarget.depth.format;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = vr->multisampleTarget.depth.format;

        // pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = vr->pipelineLayout;
        // pipelineInfo.renderPass = vr->renderPass;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pRasterizationState = &rasterization;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pViewportState = &viewport;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamic;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pNext = &pipelineRenderingCreateInfo;

        VkPipeline pipeline = {};

        // default pipeline with back-face culling
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
        vr->pipelines[prefix] = pipeline;

        // double sided
        rasterization.cullMode = VK_CULL_MODE_NONE;
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
        vr->pipelines[prefix + "_double_sided"] = pipeline;

        // alpha blending
        rasterization.cullMode = VK_CULL_MODE_NONE;
        blendAttachment.blendEnable = VK_TRUE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
        vr->pipelines[prefix + "_alpha_blending"] = pipeline;

        for (auto shaderStage : shaderStages)
            vkDestroyShaderModule(vr->device, shaderStage.module, nullptr);
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

        VkCR(vkCreateDescriptorPool(vr->device, &poolInfo, nullptr, &vr->imguiDescriptorPool));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = vr->instance;
        initInfo.DescriptorPool = vr->imguiDescriptorPool;
        // initInfo.RenderPass = vr->renderPass;
        initInfo.PipelineCache = vr->pipelineCache;
        initInfo.Device = vr->device;
        initInfo.PhysicalDevice = vr->physicalDevice;
        initInfo.QueueFamily = vr->graphicsFamily.value();
        initInfo.Queue = vr->graphicsQueue;
        initInfo.ImageCount = vs->frameOverlap;
        initInfo.MinImageCount = 2;
        initInfo.MSAASamples = vs->multiSampling ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT;;
        initInfo.Allocator = VK_NULL_HANDLE;

        initInfo.UseDynamicRendering = true;
        // dynamic rendering parameters for imgui to use
        initInfo.PipelineRenderingCreateInfo = {};
        initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vr->swapchain.colorTarget.format;

        ImGui_ImplVulkan_Init(&initInfo);

        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        vkCreateSampler(vr->device, &sampler, nullptr, &vr->viewport.sampler);

        // viewport images
        vr->viewport.images.resize(vr->swapchain.imageCount);
        vr->viewport.textureIDs.resize(vr->swapchain.imageCount);
        for (uint32_t i = 0; i < vr->viewport.images.size(); i++)
        {
            vr->viewport.format = VK_FORMAT_B8G8R8A8_SRGB;
            VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

            VkImageUsageFlags usages{};
            usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            usages |= VK_IMAGE_USAGE_SAMPLED_BIT;
            usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            vr->viewport.images[i] =
                VulkanImage::Create({ vr->swapchain.extent.width, vr->swapchain.extent.height, 1 }, vr->viewport.format, samples, usages, 1, VK_IMAGE_TYPE_2D);

            vr->viewport.textureIDs[i] =
                ImGui_ImplVulkan_AddTexture(vr->viewport.sampler, vr->viewport.images[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }

    // TODO: maybe refractor similar to dynamic rendering ex
    void VulkanBase::Render()
    {
        DrawFrame();
    }

    void VulkanBase::DrawFrame()
    {
        if (!vr->prepared) return;

        auto tStart = std::chrono::high_resolution_clock::now();

        FrameData& frame = vr->frames[vr->currentFrame];

        VkCR(vkWaitForFences(vr->device, 1, &frame.waitFence, VK_TRUE, UINT64_MAX));

        uint32_t scImageIndex;
        VkResult result = vkAcquireNextImageKHR(vr->device, vr->swapchain.swapchain, UINT64_MAX, frame.presentSemaphore, VK_NULL_HANDLE, &scImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            WindowResize();
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            DK_CORE_ASSERT(result);
        }

        VkImage msColorTarget = vr->multisampleTarget.color.image;
        VkFormat msColorTargetFormat = vr->multisampleTarget.color.format;

        VkImage msDepthTarget = vr->multisampleTarget.depth.image;
        VkFormat msDepthTargetFormat = vr->multisampleTarget.depth.format;

        VkImage viewportImage = vr->viewport.images[scImageIndex].image;
        VkFormat viewportFormat = vr->viewport.format;

        VkImage scColorTarget = vr->swapchain.colorTarget.image;
        VkFormat scColorTargetFormat = vr->swapchain.colorTarget.format;

        VkImage scImage = vr->swapchain.images[scImageIndex];
        VkFormat scFormat = vr->swapchain.format;

        // after acquiring image (to avoid deadlock), manually reset the fence to the unsignaled state
        vkResetFences(vr->device, 1, &frame.waitFence);

        VkCR(vkResetCommandBuffer(frame.commandBuffer, 0));

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VkCR(vkBeginCommandBuffer(frame.commandBuffer, &commandBufferBeginInfo));

        VulkanImage::Transition(frame.commandBuffer, msColorTarget, msColorTargetFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VulkanImage::Transition(frame.commandBuffer, msDepthTarget, msDepthTargetFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        VulkanImage::Transition(frame.commandBuffer, viewportImage, viewportFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        DrawViewport(frame.commandBuffer, scImageIndex);

        VulkanImage::Transition(frame.commandBuffer, viewportImage, viewportFormat, 1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VulkanImage::Transition(frame.commandBuffer, scColorTarget, scColorTargetFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        VulkanImage::Transition(frame.commandBuffer, scImage, scFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        DrawImGui(frame.commandBuffer, scImageIndex);

        VulkanImage::Transition(frame.commandBuffer, scImage, scFormat, 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        VkCR(vkEndCommandBuffer(frame.commandBuffer));

        UpdateUniforms();

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

        VkCR(vkQueueSubmit2KHR(vr->graphicsQueue, 1, &submitInfo, frame.waitFence));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &frame.renderSemaphore;
        VkSwapchainKHR swapChains[] = { vr->swapchain.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &scImageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(vr->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR /*|| s_State.framebufferResized*/)
        {
            WindowResize();
            return;
        }
        else if (result != VK_SUCCESS)
        {
            DK_CORE_ASSERT(result);
        }

        vr->currentFrame = (vr->currentFrame + 1) % vs->frameOverlap;

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        float frameTimer = (float)tDiff / 1000.0f;
        vr->camera.update(frameTimer);

        // TODO
        // if (!vs->animationPaused)
        // {
        //     if ((vr->models.animate) && (vr->models.scene.animations.size() > 0))
        //     {
        //         vr->models.animationTimer += frameTimer;
        //         if (vr->models.animationTimer > vr->models.scene.animations[vr->models.animationIndex].end)
        //             vr->models.animationTimer -= vr->models.scene.animations[vr->models.animationIndex].end;

        //         vr->models.scene.UpdateAnimation(vr->models.animationIndex, vr->models.animationTimer);
        //     }

        //     UpdateShaderParams();
        // }

        if (vr->camera.updated) UpdateUniforms();
    }

    void VulkanBase::DrawViewport(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        AllocatedImage& colorTarget = vr->multisampleTarget.color;
        AllocatedImage& depthTarget = vr->multisampleTarget.depth;
        AllocatedImage& viewportImage = vr->viewport.images[imageIndex];

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        colorAttachment.resolveImageView = viewportImage.view;
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingAttachmentInfo depthStencilAttachment = {};
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthStencilAttachment.pNext = nullptr;
        depthStencilAttachment.imageView = depthTarget.view;
        depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, colorTarget.extent.width, colorTarget.extent.height };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = &depthStencilAttachment;
        renderInfo.pStencilAttachment = &depthStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        VkViewport viewport{};
        viewport.width = colorTarget.extent.width;
        viewport.height = colorTarget.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = { colorTarget.extent.width,  colorTarget.extent.height };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        uint32_t dynamicOffset = 0;

        if (vs->displayBackground)
        {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->pipelineLayout, 0, 1, &vr->descriptorSets[vr->currentFrame].skybox, 1, &dynamicOffset);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->pipelines["skybox"]);
            vr->environmentModels["Skybox"]->Draw(commandBuffer);
        }

        VkDeviceSize offsets[1] = { 0 };

        uint32_t index = 0;
        for (auto& [tag, model] : vr->propModels)
        {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->vertices.buffer, offsets);

            if (model->indices.buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(commandBuffer, model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

            vr->boundPipeline = VK_NULL_HANDLE;

            // one dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
            dynamicOffset = index * static_cast<uint32_t>(vr->dynamicUniformAlignment);

            // opaque primitives first
            for (auto node : model->nodes)
                RenderNode(node, commandBuffer, Material::ALPHAMODE_OPAQUE, dynamicOffset);
            // alpha masked primitives
            for (auto node : model->nodes)
                RenderNode(node, commandBuffer, Material::ALPHAMODE_MASK, dynamicOffset);
            // transparent primitives
            // TODO: Correct depth sorting
            for (auto node : model->nodes)
                RenderNode(node, commandBuffer, Material::ALPHAMODE_BLEND, dynamicOffset);

            index++;
        }

        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanBase::DrawImGui(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = vr->swapchain.colorTarget.view;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        colorAttachment.resolveImageView = vr->swapchain.views[imageIndex];
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, vr->swapchain.extent.width, vr->swapchain.extent.height };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = nullptr;
        renderInfo.pStencilAttachment = nullptr;

        vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

        LayerStack& layerStack = Application::Get().GetLayerStack();
        ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();

        imguiLayer->Begin();
        for (Layer* layer : layerStack)
            layer->OnImGuiRender((ImTextureID)vr->viewport.textureIDs[imageIndex]);
        imguiLayer->End(commandBuffer);

        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanBase::UpdateShaderParams()
    {
        vr->shaderValuesParams.lightDir = glm::vec4(
            sin(glm::radians(vr->lightSource.rotation.x)) * cos(glm::radians(vr->lightSource.rotation.y)),
            sin(glm::radians(vr->lightSource.rotation.y)),
            cos(glm::radians(vr->lightSource.rotation.x)) * cos(glm::radians(vr->lightSource.rotation.y)),
            0.0f);
    }

    void VulkanBase::WindowResize()
    {
        if (!vr->prepared) return;

        vr->prepared = false;

        VulkanBase::Idle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(vr->device, vr->imguiDescriptorPool, nullptr);
        vkDestroySampler(vr->device, vr->viewport.sampler, nullptr);

        for (auto image : vr->viewport.images)
            VulkanImage::Destroy(image);

        VulkanImage::Destroy(vr->swapchain.colorTarget);
        VulkanImage::Destroy(vr->multisampleTarget.color);
        VulkanImage::Destroy(vr->multisampleTarget.depth);

        vkDestroyPipelineCache(vr->device, vr->pipelineCache, nullptr);

        VulkanBase::Idle();

        SetUpSwapchain();

        UpdateUniforms();

        SetUpImGui();

        ImGuiLayer::SetStyles();

        VulkanBase::Idle();

        vr->prepared = true;
    }

    void VulkanBase::ViewportResize(const glm::vec2& viewportSize)
    {
        vr->camera.updateAspectRatio((float)viewportSize.x / (float)viewportSize.y);
    }

}
