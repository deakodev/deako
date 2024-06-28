#include "VulkanBase.h"
#include "dkpch.h"

#include "VulkanDebug.h"

namespace Deako {

    VkInstance VulkanBase::s_Instance{ VK_NULL_HANDLE };
    std::vector<const char*> VulkanBase::s_Extensions;
    std::vector<const char*> VulkanBase::s_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
    VulkanSettings VulkanBase::s_Settings;

    void VulkanBase::Init()
    {
        CreateInstance();

        if (s_Settings.validation)
            VulkanDebugMessenger::Create();
    }

    void VulkanBase::CleanUp()
    {
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

}
