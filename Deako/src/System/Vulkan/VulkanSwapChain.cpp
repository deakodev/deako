#include "VulkanSwapChain.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

#include "VulkanBase.h"
#include "VulkanFramebuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanCommand.h"

#include <GLFW/glfw3.h>

namespace Deako {

    SwapChainSupportDetails VulkanSwapChain::s_Details;

    std::vector<VkImage> VulkanSwapChain::s_Images;
    std::vector<VkImageView> VulkanSwapChain::s_ImageViews;

    void VulkanSwapChain::Create()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        s_Details = QuerySupport(vr->physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(s_Details.formats);
        vr->imageFormat = surfaceFormat.format;
        VkPresentModeKHR presentMode = ChoosePresentMode(s_Details.presentModes);
        vr->imageExtent = ChooseExtent(s_Details.capabilities);

        // Recommended to request at least 1 more image than the min
        uint32_t imageCount = s_Details.capabilities.minImageCount + 1;
        // Check to make sure we dont exceed the max number of possible images
        if (s_Details.capabilities.maxImageCount > 0 && imageCount > s_Details.capabilities.maxImageCount)
            imageCount = s_Details.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = vr->surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = vr->imageFormat;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = vr->imageExtent;
        createInfo.imageArrayLayers = 1; // Always 1 except stereoscopic 3D apps
        // Specifies the kind of operations the images in the swap chain are for. Eg. color attachment for direct rendering and 'transfer' for post processing using the memory operation to transfer the rendered image
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        /* Specify how to handle swap chain images used across multiple queue families
                We’ll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue. Two ways to handle images that are accessed from multiple queues:
                       • VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family (best performance)
                       • VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers
                */

        if (s_Details.capabilities.supportedTransforms & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        // Enable transfer destination on swap chain images if supported
        if (s_Details.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        uint32_t queueFamilyIndices[] = { vr->graphicsFamily.value(), vr->presentFamily.value() };

        if (vr->graphicsFamily != vr->presentFamily)
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

        VkResult result = vkCreateSwapchainKHR(vr->device, &createInfo, nullptr, &vr->swapChain);
        DK_CORE_ASSERT(!result);

        // Images were automatically created during swap chain creation and they will be automatically cleaned up once swap chain is destroyed, we just need to set them to m_Images
        vkGetSwapchainImagesKHR(vr->device, vr->swapChain, &imageCount, nullptr);
        s_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(vr->device, vr->swapChain, &imageCount, s_Images.data());

        CreateImageViews(vr->device);
    }

    void VulkanSwapChain::Recreate()
    {
        VulkanResources* vr = VulkanBase::GetResources();

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

        vkDeviceWaitIdle(vr->device);

        vr->depthAttachment.reset();
        VulkanFramebufferPool::CleanUp();
        // CleanUpViewport();
        CleanUp();

        CreateSurface();
        Create();
        // VulkanSwapChain::CreateViewportImages();
        // VulkanSwapChain::CreateViewportImageViews();
        DepthAttachment::Create();
        VulkanFramebufferPool::Create();

        // TODO: not sure if this is needed
        Application::Get().GetImGuiLayer()->SetViewportTextureIDs();
    }

    void VulkanSwapChain::CreateSurface()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

        VkResult result = glfwCreateWindowSurface(vr->instance, window, nullptr, &vr->surface);
        DK_CORE_ASSERT(!result);
    }

    void VulkanSwapChain::CreateImageViews(VkDevice device)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        s_ImageViews.resize(s_Images.size());

        for (size_t i = 0; i < s_Images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = s_Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Options are 1D, 2D, 3D, or cube maps
            createInfo.format = vr->imageFormat;
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
            DK_CORE_ASSERT(!result);
        }
    }

    void VulkanSwapChain::CreateViewportImages()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        vr->viewportImages.resize(s_Images.size());
        vr->viewportImageMemory.resize(s_Images.size());

        for (uint32_t i = 0; i < vr->viewportImages.size(); i++)
        {
            VkImageCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
            createInfo.extent.width = vr->imageExtent.width;
            createInfo.extent.height = vr->imageExtent.height;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers = 1;
            createInfo.mipLevels = 1;
            createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            createInfo.tiling = VK_IMAGE_TILING_LINEAR;
            createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            vkCreateImage(vr->device, &createInfo, nullptr, &vr->viewportImages[i]);

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(vr->device, vr->viewportImages[i], &memRequirements);

            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAllocInfo.allocationSize = memRequirements.size;
            memAllocInfo.memoryTypeIndex = Buffer::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            vkAllocateMemory(vr->device, &memAllocInfo, nullptr, &vr->viewportImageMemory[i]);
            vkBindImageMemory(vr->device, vr->viewportImages[i], vr->viewportImageMemory[i], 0);

            InsertImageMemoryBarrier(
                vr->viewportCommandPool,
                vr->viewportImages[i],
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
            );
        }
    }

    void VulkanSwapChain::InsertImageMemoryBarrier(
        VkCommandPool commandPool,
        VkImage image,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkImageSubresourceRange subresourceRange)
    {
        VkCommandBuffer commandBuffer = VulkanCommandPool::BeginSingleTimeCommands(commandPool);

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(
            commandBuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

        VulkanCommandPool::EndSingleTimeCommands(commandPool, commandBuffer);
    }


    void VulkanSwapChain::CreateViewportImageViews()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        vr->viewportImageViews.resize(s_Images.size());

        for (uint32_t i = 0; i < vr->viewportImageViews.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = vr->viewportImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(vr->device, &createInfo, nullptr, &vr->viewportImageViews[i]);
            DK_CORE_ASSERT(result == VK_SUCCESS);
        }
    }

    void VulkanSwapChain::CleanUp()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        for (auto imageView : s_ImageViews)
            vkDestroyImageView(vr->device, imageView, nullptr);

        vkDestroySwapchainKHR(vr->device, vr->swapChain, nullptr);
        vkDestroySurfaceKHR(vr->instance, vr->surface, nullptr);
    }

    void VulkanSwapChain::CleanUpViewport()
    {
        VulkanResources* vr = VulkanBase::GetResources();

        for (auto image : vr->viewportImages)
            vkDestroyImage(vr->device, image, nullptr);
        for (auto imageView : vr->viewportImageViews)
            vkDestroyImageView(vr->device, imageView, nullptr);
        for (auto imageMemory : vr->viewportImageMemory)
            vkFreeMemory(vr->device, imageMemory, nullptr);
    }

    SwapChainSupportDetails VulkanSwapChain::QuerySupport(VkPhysicalDevice device)
    {
        VulkanResources* vr = VulkanBase::GetResources();

        // Swap chain capaabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vr->surface, &s_Details.capabilities);

        // Swap chain formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vr->surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            s_Details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, vr->surface, &formatCount, s_Details.formats.data());
        }

        // Swap chain present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vr->surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            s_Details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, vr->surface, &presentModeCount, s_Details.presentModes.data());
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
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
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
