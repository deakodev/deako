#include "VulkanDevice.h"
#include "dkpch.h"

#include "VulkanSwapChain.h"

namespace Deako {

    Ref<VulkanResources> Device::s_VR = VulkanBase::GetResources();
    Ref<VulkanSettings> Device::s_VS = VulkanBase::GetSettings();

    void Device::Create()
    {
        DeterminePhysical();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            s_VR->graphicsFamily.value(),
            s_VR->presentFamily.value()
        };

        // Between 0.0 - 1.0; influences the scheduling of command buffer execution
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            // Don’t really need more than one queue per family; can create all of the command buffers on multiple threads and then submit them all at once on the main thread with a single low-overhead call
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Features that we queried with vkGetPhysicalDeviceFeatures
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        // Enable required device extensions
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(s_VS->deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = s_VS->deviceExtensions.data();

        if (s_VS->validation)
        {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(s_VS->validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = s_VS->validationLayers.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        // note - queues are automatically created here
        VkResult result = vkCreateDevice(s_VR->physicalDevice, &deviceCreateInfo, nullptr, &s_VR->device);
        DK_CORE_ASSERT(!result);

        vkGetDeviceQueue(s_VR->device, s_VR->graphicsFamily.value(), 0, &s_VR->graphicsQueue);
        vkGetDeviceQueue(s_VR->device, s_VR->presentFamily.value(), 0, &s_VR->presentQueue);
    }

    void Device::CleanUp()
    {
        vkDestroyDevice(s_VR->device, nullptr);
    }

    void Device::DeterminePhysical()
    {
        // First check if any devices are supported
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(s_VR->instance, &deviceCount, nullptr);

        DK_CORE_ASSERT(deviceCount);

        // Allocate an array to hold all of the VkPhysicalDevice handles
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(s_VR->instance, &deviceCount, devices.data());

        std::multimap<int, VkPhysicalDevice> candidates;

        // Determine scores of each device
        for (const auto& device : devices)
        {
            int score = RatePhysical(device);
            candidates.insert(std::make_pair(score, device));
        }

        // Determine best candidate and set as physical device
        if (candidates.rbegin()->first > 0)
            s_VR->physicalDevice = candidates.rbegin()->second;
        else
            DK_CORE_ASSERT(false);

        // Set family indices of our selected physical device
        FindQueueFamilies(s_VR->physicalDevice);
    }

    int Device::RatePhysical(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        int score = 0;
        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;

        // Max possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        if (!deviceFeatures.geometryShader) // App can't function without geometry shaders
            score += 1000;

        // Look for families, must have required families
        FindQueueFamilies(device);
        bool queueFamiliesFound = s_VR->graphicsFamily.has_value() && s_VR->presentFamily.has_value();

        // Must support required extensions and features
        bool extensionsSupported = CheckDeviceExtensionSupport(device);
        bool featuresSupported = deviceFeatures.samplerAnisotropy;

        // Must have adequate swap chain details
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = SwapChain::QuerySupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty()
                && !swapChainSupport.presentModes.empty();
        }

        bool suitable = extensionsSupported && featuresSupported && swapChainAdequate && queueFamiliesFound;

        return suitable ? score : 0;
    }

    void Device::FindQueueFamilies(VkPhysicalDevice device)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // Allocate an array to hold all of the queue families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // Find queue families that are supported
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                s_VR->graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, s_VR->surface, &presentSupport);

            if (presentSupport)
                s_VR->presentFamily = i;

            if (s_VR->graphicsFamily.has_value() && s_VR->presentFamily.has_value())
                break;

            i++;
        }
    }

    bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(s_VS->deviceExtensions.begin(), s_VS->deviceExtensions.end());

        // checks if available extension array satisfies the required extensions array
        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        // if empty then required device extensions are supported
        return requiredExtensions.empty();
    }

}
