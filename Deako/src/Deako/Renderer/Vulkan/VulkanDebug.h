#pragma once

#include <vulkan/vulkan.h>

namespace Deako {
    namespace VulkanDebug {

        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);

        void DestroyDebugUtilsMessengerEXT();

    }

    std::string ErrorString(VkResult errorCode);

    #define VkCR(f)																				            \
    {																										\
        VkResult res = (f);																					\
        if (res != VK_SUCCESS)																				\
        {																									\
            std::cout << "Fatal : VkResult is \"" << ErrorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n";                                                                                           \
            assert(res == VK_SUCCESS);																		\
        }																									\
    }
}


