#pragma once

#include "VulkanTypes.h"
#include "VulkanModel.h"

#include <vulkan/vulkan.h>

namespace Deako {

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

        uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);

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

        AllocatedImage Create(VkExtent3D extent, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, uint32_t mipLevels = 1, VkImageType imageType = VK_IMAGE_TYPE_2D);

        void Destroy(const AllocatedImage& allocImage);

        void TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, uint32_t mipLevels, VkImageLayout currentLayout, VkImageLayout newLayout);

    } // end namespace VulkanImage
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanBuffer {

        AllocatedBuffer Create(size_t allocSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqFlags = 0);

        void Destroy(const AllocatedBuffer& buffer);

    } // end namespace VulkanBuffer
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanCommand {

        VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool);

        void EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer);

    } // end namespace VulkanImage
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanLoad {

        void Scene(std::string filename);

    } // end namespace VulkanLoad
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanShader {

        std::vector<char> ReadShaderFile(const std::string& filename);

        VkShaderModule CreateShaderModule(const std::string& filename);

    } // end namespace VulkanShader
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


    void GenerateBRDFLookUpTable();


}
