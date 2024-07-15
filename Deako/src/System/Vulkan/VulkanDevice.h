#pragma once

#include "VulkanBase.h"

#include <vulkan/vulkan.h>

namespace Deako {

    class Device
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
        static Ref<VulkanResources> s_VR;
        static Ref<VulkanSettings> s_VS;
    };

}
