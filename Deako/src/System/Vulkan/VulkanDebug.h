#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    // Can have multiple, but class structured for one for now
    class DebugMessenger
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
        static Ref<VulkanResources> s_VR;
    };

}
