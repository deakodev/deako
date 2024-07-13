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
        static void CreateViewportImages();
        static void CreateViewportImageViews();
        static void CleanUp();
        static void CleanUpViewport();

        static const std::vector<VkImageView>& GetImageViews() { return s_ImageViews; }
        static SwapChainSupportDetails QuerySupport(VkPhysicalDevice device);

    private:
        static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);
        static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        static void InsertImageMemoryBarrier(VkCommandPool commandPool, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

    private:
        static SwapChainSupportDetails s_Details;
        static std::vector<VkImage> s_Images;
        static std::vector<VkImageView> s_ImageViews;
    };

}
