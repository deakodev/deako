#include "VulkanSwapChain.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "Deako/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Deako {

    VkSwapchainKHR VulkanSwapChain::s_SwapChain{ VK_NULL_HANDLE };
    VkSurfaceKHR VulkanSwapChain::s_Surface{ VK_NULL_HANDLE };
    SwapChainSupportDetails VulkanSwapChain::s_Details;
    VkFormat VulkanSwapChain::s_Format;
    VkExtent2D VulkanSwapChain::s_Extent;
    std::vector<VkImage> VulkanSwapChain::s_Images;
    std::vector<VkImageView> VulkanSwapChain::s_ImageViews;

    void VulkanSwapChain::Create()
    {
        VkPhysicalDevice physicalDevice = VulkanDevice::GetPhysical();
        VkDevice logicalDevice = VulkanDevice::GetLogical();

        s_Details = QuerySupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(s_Details.formats);
        s_Format = surfaceFormat.format;
        VkPresentModeKHR presentMode = ChoosePresentMode(s_Details.presentModes);
        s_Extent = ChooseExtent(s_Details.capabilities);

        // Recommended to request at least 1 more image than the min
        uint32_t imageCount = s_Details.capabilities.minImageCount + 1;
        // Check to make sure we dont exceed the max number of possible images
        if (s_Details.capabilities.maxImageCount > 0 && imageCount > s_Details.capabilities.maxImageCount)
            imageCount = s_Details.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = s_Surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = s_Format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = s_Extent;
        createInfo.imageArrayLayers = 1; // Always 1 except stereoscopic 3D apps
        // Specifies the kind of operations the images in the swap chain are for. Eg. color attachment for direct rendering and 'transfer' for post processing using the memory operation to transfer the rendered image
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        /* Specify how to handle swap chain images used across multiple queue families
                We’ll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue. Two ways to handle images that are accessed from multiple queues:
                       • VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family (best performance)
                       • VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers
                */
        const QueueFamilyIndices& indices = VulkanDevice::GetQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        // Specify if transform should be applied to images in the swap chain if supported (supportedTransforms in capabilities), eg. 90deg rotation or horizontal flip
        createInfo.preTransform = s_Details.capabilities.currentTransform; // Default
        // Specifies if alpha channel should be used for blending with other windows in window system
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Default
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        // Possible that the swap chain becomes invalid while app is running, eg. window resized. In that case the swap chain needs to be recreated from scratch and a reference to the old one must be specified in this field
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &s_SwapChain);
        DK_CORE_ASSERT(!result, "Failed to create swap chain!");

        // Images were automatically created during swap chain creation and they will be automatically cleaned up once swap chain is destroyed, we just need to set them to m_Images
        vkGetSwapchainImagesKHR(logicalDevice, s_SwapChain, &imageCount, nullptr);
        s_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice, s_SwapChain, &imageCount, s_Images.data());

        CreateImageViews(logicalDevice);
    }

    void VulkanSwapChain::Recreate()
    {
        VkDevice device = VulkanDevice::GetLogical();
        auto& window = Application::Get().GetWindow();

        auto [width, height] = window.GetWindowFramebufferSize();
        while (width == 0 || height == 0)
        {
            // TODO: look at this again, doesnt work on MacOS
            auto [newWidth, newHeight] = window.GetWindowFramebufferSize(); // when unminimized break loop
            width = newWidth;
            height = newHeight;
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        // TODO: pg. 140 may need to seperate swap chain framebuffer portion if adding different types of framebuffers so we dont accidently CleanUp the wrong framebuffers
        VulkanFramebufferPool::CleanUp();
        CleanUp();

        CreateSurface();
        Create();
        VulkanFramebufferPool::Create(); // and framebuffers depend directly on the image views
    }

    void VulkanSwapChain::CreateSurface()
    {
        VkInstance instance = VulkanBase::GetInstance();
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

        VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &s_Surface);
        DK_CORE_ASSERT(!result, "Failed to create window surface!");
    }

    void VulkanSwapChain::CreateImageViews(VkDevice device)
    {
        s_ImageViews.resize(s_Images.size());

        for (size_t i = 0; i < s_Images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = s_Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Options are 1D, 2D, 3D, or cube maps
            createInfo.format = s_Format;
            // Default mapping. But you can swizzle the color channels around. Eg. map all of the channels to the red channel for a monochrome texture. Eg. map constant values of 0 and 1 to a channel
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            // Describes what the image’s purpose is and which part of the image should be accessed
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(device, &createInfo, nullptr, &s_ImageViews[i]);
            DK_CORE_ASSERT(!result, "Failed to create image views!");
        }
    }

    void VulkanSwapChain::CleanUp()
    {
        VkInstance instance = VulkanBase::GetInstance();
        VkDevice device = VulkanDevice::GetLogical();

        for (auto imageView : s_ImageViews)
            vkDestroyImageView(device, imageView, nullptr);
        vkDestroySwapchainKHR(device, s_SwapChain, nullptr);
        vkDestroySurfaceKHR(instance, s_Surface, nullptr);
    }

    SwapChainSupportDetails VulkanSwapChain::QuerySupport(VkPhysicalDevice device)
    {
        // Swap chain capaabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, s_Surface, &s_Details.capabilities);

        // Swap chain formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, s_Surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            s_Details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, s_Surface, &formatCount, s_Details.formats.data());
        }

        // Swap chain present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, s_Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            s_Details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, s_Surface, &presentModeCount, s_Details.presentModes.data());
        }

        return s_Details;
    }

    VkSurfaceFormatKHR VulkanSwapChain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        /* Each VkSurfaceFormatKHR contains:
            • format - specifies the color channels and types. Eg. VK_FORMAT_B8G8R8A8_SRGB means to store B, G, R and alpha channels in that order with an 8 bit unsigned integer for a total of 32 bits per pixel
            • colorSpace - indicates if SRGB color space is supported or not. We'll use the VK_COLOR_SPACE_SRGB_NONLINEAR_KHR flag because it results in more accurate perceived colors
         */
        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == VK_FORMAT_R8G8B8_SRGB
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR VulkanSwapChain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        /* Four possible modes:
         • VK_PRESENT_MODE_IMMEDIATE_KHR: images submitted are transferred to screen right away, may result in tearing
         • VK_PRESENT_MODE_FIFO_KHR: swap chain is a queue where the display takes an image from the front of the queue when refreshed (“vertical blank”) and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait. Similar to "vertical sync" as found in modern games
         • VK_PRESENT_MODE_FIFO_RELAXED_KHR: differs from the previous one if the app is late and the queue was empty at the last vertical blank. Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing
         • VK_PRESENT_MODE_MAILBOX_KHR: instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones. This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync. Commonly known as “triple buffering”
         */

        for (const auto& availablePresentMode : presentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) // better than v-sync, but high energy use
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR; // guaranteed to be available
    }

    VkExtent2D VulkanSwapChain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        /* Swap extent is the resolution of the swap chain images; almost always exactly equal to the resolution of the window that we’re drawing to in pixels (more on that in a moment)
         The range of the possible resolutions is defined in the VkSurfaceCapabilitiesKHR structure.
            • Vulkan tells us to match the resolution of the window by setting width/height in currentExtent member.
            • However, some window managers do allow us to differ here and this is indicated by setting the width and height in currentExtent to a special value: the maximum value of uint32_t. In that case we’ll pick the resolution that best matches the window within the minImageExtent and maxImageExtent bounds.
         GLFW uses two units when measuring sizes:
            • pixels - vulkan works with pixels, so the swap chain extent must be
            • screen coordinates - width/height that we specified earlier when creating the window, we can't use these values because high DPI display's (like retina) have a larger density of pixels/ higher resolution than the screen coordinates
         */

        VkExtent2D actualExtent;

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            // indicates that the platform window manager does not enforce a specific swap chain
            return capabilities.currentExtent;
        }
        else
        {
            // indicates that the platform window manager does not enforce a specific swap chain extent and the app is free to set its own width/height (that are within the vulkan min/maxImageExtent bounds)
            // query the resolution of the window in pixels
            // auto [width, height] = Window::GetWindowFramebufferSize();
            // actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; TODO
        }

        // clamp width/height between the allowed min/max extents supported by the implementation
        actualExtent.width = std::clamp(actualExtent.width,
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

}
