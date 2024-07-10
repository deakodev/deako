#pragma once

#include <GLFW/glfw3.h>

namespace Deako {

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanSwapChain
    {
    public:
        static void Create();
        static void Recreate();
        static void CreateSurface();
        static void CreateImageViews(VkDevice device);
        static void CleanUp();

        static VkSwapchainKHR GetSwapChain() { return s_SwapChain; }
        static VkSurfaceKHR GetSurface() { return s_Surface; }
        static VkFormat& GetImageFormat() { return s_Format; }
        static VkExtent2D& GetExtent() { return s_Extent; }
        static const std::vector<VkImageView>& GetImageViews() { return s_ImageViews; }
        static SwapChainSupportDetails QuerySupport(VkPhysicalDevice device);

    private:
        static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);
        static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    private:
        static VkSwapchainKHR s_SwapChain;
        static VkSurfaceKHR s_Surface;
        static SwapChainSupportDetails s_Details;
        static VkFormat s_Format;
        static VkExtent2D s_Extent;

        static std::vector<VkImage> s_Images;
        static std::vector<VkImageView> s_ImageViews;
    };

}
