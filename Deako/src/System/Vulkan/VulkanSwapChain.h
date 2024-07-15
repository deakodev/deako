#pragma once

#include "VulkanBase.h"

#include <GLFW/glfw3.h>

namespace Deako {

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR                  capabilities;
        std::vector<VkSurfaceFormatKHR>           formats;
        std::vector<VkPresentModeKHR>             presentModes;
    };

    class SwapChain
    {
    public:
        static void Create();
        static void Recreate();
        static void CreateSurface();
        static void CreateImageViews();
        static void CleanUp();
        static void CleanUpSurface();

        static SwapChainSupportDetails QuerySupport(VkPhysicalDevice device);

    private:
        static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);
        static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    private:
        static SwapChainSupportDetails             s_Details;
        static Ref<VulkanResources>                s_VR;
    };

}
