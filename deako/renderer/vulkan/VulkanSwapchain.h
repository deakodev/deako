#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

	// forward declare
	class VulkanInstance;
	class VulkanSurface;
	class VulkanDevice;

	struct SwapchainSupportDetails
	{
		std::vector<VkFormat> Formats;
		std::vector<VkColorSpaceKHR> ColorSpaces;
		std::vector<VkPresentModeKHR> PresentModes;
		VkSurfaceCapabilitiesKHR Capabilities;
	};

	struct SwapchainConfig
	{
		uint32_t ImageCount;
		VkImageUsageFlags Usage;
		VkFormat Format;
		VkColorSpaceKHR ColorSpace;
		VkPresentModeKHR PresentMode;
		VkExtent2D Extent;
	};

	/// VulkanSwapchain encapsulates a Vulkan swapchain
	class VulkanSwapchain : public Inheritor<Object, VulkanSwapchain>
	{
	public:
		VulkanSwapchain(Ref<VulkanDevice> device, Ref<VulkanSurface> surface, Ref<VulkanSwapchain> oldSwapchain, const SwapchainConfig* config = nullptr);
		~VulkanSwapchain();

		void Clear();

		const SwapchainConfig& GetConfig() const { return m_Config; }

		operator VkSwapchainKHR() const { return m_Swapchain; }
		const VkSwapchainKHR& Vk() const { return m_Swapchain; }

	private:
		VkSwapchainKHR m_Swapchain;

		Ref<VulkanDevice> m_Device;
		Ref<VulkanSurface> m_Surface;

		SwapchainConfig m_Config;
	};

	DK_TYPE_NAME(VulkanSwapchain);

	SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkFormat SelectSwapchainFormat(const std::vector<VkFormat>& formatsPreferred, const std::vector<VkFormat>& formatsSupported);
	VkColorSpaceKHR SelectSwapchainColorSpace(VkColorSpaceKHR colorSpacePreferred, const std::vector<VkColorSpaceKHR>& colorSpacesSupported);
	VkPresentModeKHR SelectSwapchainPresentMode(VkPresentModeKHR presentModePreferred, const std::vector<VkPresentModeKHR>& presentModesSupported);
	VkExtent2D SelectSwapchainExtent(uint32_t width, uint32_t height, const VkSurfaceCapabilitiesKHR& capabilities);
	uint32_t SelectSwapchainImageCount(uint32_t preferredImageCount, const VkSurfaceCapabilitiesKHR& capabilities);

}

