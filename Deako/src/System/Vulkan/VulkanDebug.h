#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    // Can have multiple, but class structured for one for now
    class VulkanDebugMessenger
    {
    public:
        static void Create();
        static void CleanUp();

        static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    private:
        static VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);
        static void DestroyDebugUtilsMessengerEXT();

    private:
        static VkDebugUtilsMessengerEXT s_DebugMessenger;
    };

}
