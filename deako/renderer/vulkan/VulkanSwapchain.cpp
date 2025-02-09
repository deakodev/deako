#include "deako_pch.h"
#include "VulkanSwapchain.h"

#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"

namespace Deako {

	VulkanSwapchain::VulkanSwapchain(Ref<VulkanDevice> device, Ref<VulkanSurface> surface, Ref<VulkanSwapchain> oldSwapchain, const SwapchainConfig* config)
		: m_Swapchain(VK_NULL_HANDLE), m_Device(device), m_Surface(surface)
	{
		DK_CORE_ASSERT(config || oldSwapchain, "VulkanSwapchain requires a config or old swapchain config!");

		//if (config)
		//{
		//	m_Config = *config;
		//}
		//else if (oldSwapchain)
		//{
		//	m_Config = oldSwapchain->GetConfig();
		//}

		//VkSwapchainCreateInfoKHR createInfo = {};
		//createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		//createInfo.surface = m_Surface->Vk();

		//createInfo.minImageCount = m_Config.ImageCount;
		//createInfo.imageUsage = m_Config.Usage;
		//createInfo.imageFormat = m_Config.Format;
		//createInfo.imageColorSpace = m_Config.ColorSpace;
		//createInfo.imageExtent = m_Config.Extent;
		//createInfo.imageArrayLayers = 1;


		//VkSwapchainKHR newSwapchain;
		//VK_CHECK(vkCreateSwapchainKHR(m_Device->Vk(), &createInfo, nullptr, &newSwapchain));

		//m_Swapchain = newSwapchain;
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		/*if (m_Swapchain)
		{
			vkDestroySwapchainKHR(m_Device->Vk(), m_Swapchain, nullptr);
		}
		DK_CORE_INFO("VulkanSwapchain destroyed!");*/
	}

	void VulkanSwapchain::Clear()
	{
		// TODO
	}

	SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapchainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		if (formatCount != 0)
		{
			surfaceFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats.data());

			for (const auto& surfaceFormat : surfaceFormats)
			{
				details.Formats.push_back(surfaceFormat.format);
				details.ColorSpaces.push_back(surfaceFormat.colorSpace);
			}
		}

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

	VkFormat SelectSwapchainFormat(const std::vector<VkFormat>& formatsPreferred, const std::vector<VkFormat>& formatsSupported)
	{
		if (formatsPreferred.empty() || formatsSupported.empty() || formatsSupported[0] == VK_FORMAT_UNDEFINED)
		{
			return VK_FORMAT_B8G8R8A8_UNORM; // fallback
		}

		for (auto formatPreferred : formatsPreferred)
		{
			for (auto format : formatsSupported)
			{
				if (formatPreferred == format)
				{
					return formatPreferred;
				}
			}
		}

		return formatsSupported[0]; // fallback
	}

	VkColorSpaceKHR SelectSwapchainColorSpace(VkColorSpaceKHR colorSpacePreferred, const std::vector<VkColorSpaceKHR>& colorSpacesSupported)
	{
		for (auto colorSpace : colorSpacesSupported)
		{
			if (colorSpacePreferred == colorSpace)
			{
				return colorSpacePreferred;
			}
		}

		for (auto colorSpace : colorSpacesSupported) // fallback
		{
			if (VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == colorSpace)
			{
				return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			}
		}

		return colorSpacesSupported[0]; // fallback
	}

	VkPresentModeKHR SelectSwapchainPresentMode(VkPresentModeKHR presentModePreferred, const std::vector<VkPresentModeKHR>& presentModesSupported)
	{
		for (auto presentMode : presentModesSupported)
		{
			if (presentModePreferred == presentMode)
			{
				return presentModePreferred;
			}
		}

		for (auto presentMode : presentModesSupported) // fallback
		{
			if (VK_PRESENT_MODE_MAILBOX_KHR == presentMode)
			{
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}

		return presentModesSupported[0]; // fallback
	}

	VkExtent2D SelectSwapchainExtent(uint32_t width, uint32_t height, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		/*if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D extent;
			extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
			extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));
			return extent;
		}*/
		return { 0, 0 };
	}

	uint32_t SelectSwapchainImageCount(uint32_t preferredImageCount, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		/*uint32_t imageCount = std::max(preferredImageCount, details.capabilities.minImageCount);

		if (details.capabilities.maxImageCount > 0)
			return imageCount = std::min(imageCount, details.capabilities.maxImageCount);

		return imageCount;*/
		return 0;
	}


}