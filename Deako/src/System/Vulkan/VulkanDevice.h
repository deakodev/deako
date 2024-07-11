#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    class VulkanDevice
    {
    public:
        static void Create();
        static void CleanUp();

    private:
        static void DeterminePhysical();
        static int RatePhysical(VkPhysicalDevice device);
        static void FindQueueFamilies(VkPhysicalDevice device);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    private:
        static const std::vector<const char*> s_DeviceExtensions;
    };

}
