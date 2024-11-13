#pragma once

#include "Deako/Renderer/RendererTypes.h"

#include <vulkan/vulkan.h>

namespace Deako {

    struct SwapchainDetails
    {
        VkSurfaceCapabilitiesKHR                  capabilities;
        std::vector<VkSurfaceFormatKHR>           formats;
        std::vector<VkPresentModeKHR>             presentModes;
    };

    struct AllocatedImage
    {
        VkImage                                   image;
        VkImageView                               view;
        VkDeviceMemory                            memory;
        VkExtent3D                                extent;
        VkFormat                                  format;
    };

    struct AllocatedBuffer
    {
        VkBuffer                                  buffer;
        VkDeviceMemory                            memory;
        VkMemoryRequirements                      memReqs;
        void* /*                               */ mapped;
    };

    struct UniformBuffer
    {
        AllocatedBuffer                           buffer;
        VkDescriptorBufferInfo                    descriptor;
    };

    namespace VulkanSwapchain {

        SwapchainDetails QuerySupport(VkPhysicalDevice physicalDevice);

        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);

        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    } // end namespace VulkanSwapchain
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


    namespace VulkanDevice {

        void FindQueueFamilies(VkPhysicalDevice physicalDevice);

        bool CheckExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*> extensions);

        int RatePhysical(VkPhysicalDevice physicalDevice);

        DkU32 GetMemoryType(DkU32 typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);

    } // end namespace VulkanDevice
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


    namespace VulkanDepth {

        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        VkFormat FindFormat();

    } // end namespace VulkanDepth
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


    namespace VulkanImage {

        AllocatedImage Create(VkExtent3D extent, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, DkU32 mipLevels = 1, VkImageType imageType = VK_IMAGE_TYPE_2D);

        void Destroy(const AllocatedImage& allocImage);

        void Transition(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, DkU32 mipLevels, VkImageLayout currentLayout, VkImageLayout newLayout);

        VkFormat ConvertToVulkanFormat(ImageFormat format);

        VkImageUsageFlags ConvertToVulkanUsage(ImageUsage usage);

        VkImageLayout ConvertToVulkanLayout(ImageLayout layout);

    } // end namespace VulkanImage
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


    namespace VulkanBuffer {

        AllocatedBuffer Create(size_t allocSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqFlags = 0);

        void Destroy(AllocatedBuffer& buffer);

    } // end namespace VulkanBuffer
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


    namespace VulkanCommand {

        VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool);

        void EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer);

    } // end namespace VulkanImage
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

    namespace VulkanShader {

        std::vector<char> ReadFile(const std::filesystem::path& path);

        VkShaderModule CreateModule(const std::filesystem::path&);

    } // end namespace VulkanShader
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

    namespace VulkanMemory {

        void* AlignedAlloc(size_t size, size_t alignment);

        void AlignedFree(void* data);

    } // end namespace VulkanMemory
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


    void GenerateBRDFLookUpTable();


}
