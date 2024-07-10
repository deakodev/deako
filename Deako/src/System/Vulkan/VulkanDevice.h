#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

    struct QueueFamilyIndices
    {
        // std::optional - wrapper containing no value until assiged one
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    class VulkanDevice
    {
    public:
        static VkDevice Create();
        static void CleanUp();

        static VkPhysicalDevice GetPhysical() { return s_PhysicalDevice; }
        static VkDevice GetLogical() { return s_LogicalDevice; }
        static QueueFamilyIndices& GetQueueFamilyIndices() { return s_QueueFamilyIndices; }
        static VkQueue GetGraphicsQueue() { return s_GraphicsQueue; }
        static VkQueue GetPresentQueue() { return s_PresentQueue; }

        static void DeterminePhysical();

    private:
        static int RatePhysical(VkPhysicalDevice device);
        static void FindQueueFamilies(VkPhysicalDevice device);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    private:
        static VkPhysicalDevice s_PhysicalDevice;
        static VkDevice s_LogicalDevice;

        static QueueFamilyIndices s_QueueFamilyIndices;
        static VkQueue s_GraphicsQueue;
        static VkQueue s_PresentQueue;

        static VkCommandPool s_CommandPool;

        static const std::vector<const char*> s_DeviceExtensions;
    };

}
