#include "deako_pch.h"
#include "VulkanResources.h"

#include "VulkanInstance.h"
#include "VulkanDebugMessenger.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

namespace Deako {

	VulkanResources::VulkanResources(Window* window)
		: m_Window(window)
	{
		SetupInstance();

		SetupSurface();

		SetupPhysicalDevice();

		SetupDevice();

		SetupSwapchain();
	}

	void VulkanResources::SetupInstance()
	{
		m_Instance = VulkanInstance::Create();
	}

	void VulkanResources::SetupSurface()
	{
		DK_CORE_ASSERT(m_Window, "VulkanSurface setup requires a valid window!");
		if (!m_Instance) { SetupInstance(); }

		m_Surface = VulkanSurface::Create(m_Window, m_Instance);
	}

	void VulkanResources::SetupPhysicalDevice()
	{
		if (!m_Surface) { SetupSurface(); }

		m_PhysicalDevice = m_Instance->SelectPhysicalDevice(m_Surface);
	}

	void VulkanResources::SetupDevice()
	{
		if (!m_PhysicalDevice) { SetupPhysicalDevice(); }

		m_Device = VulkanDevice::Create(m_Instance, m_PhysicalDevice);
	}

	void VulkanResources::SetupSwapchain()
	{
		if (!m_Device) { SetupDevice(); }

		SwapchainSupportDetails supported = QuerySwapchainSupportDetails(m_PhysicalDevice->Vk(), m_Surface->Vk());

		uint32_t imageCount = SelectSwapchainImageCount(2, supported.Capabilities);

		const std::vector<VkFormat> preferredFormats = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_A8B8G8R8_SRGB_PACK32 };
		VkFormat format = SelectSwapchainFormat(preferredFormats, supported.Formats);

		const VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		VkColorSpaceKHR colorSpace = SelectSwapchainColorSpace(preferredColorSpace, supported.ColorSpaces);

		VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		VkPresentModeKHR presentMode = SelectSwapchainPresentMode(preferredPresentMode, supported.PresentModes);

		auto [width, height] = m_Window->GetScaledSize();
		VkExtent2D extent = SelectSwapchainExtent(width, height, supported.Capabilities);

		SwapchainConfig config = {};
		config.ImageCount = imageCount;
		config.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		config.Format = format;
		config.ColorSpace = colorSpace;
		config.PresentMode = presentMode;
		config.Extent = extent;

		BuildSwapchain(&config);

	}

	void VulkanResources::BuildSwapchain(const SwapchainConfig* config)
	{
		if (m_Swapchain)
		{
			m_Device->WaitIdle();
			m_Swapchain->Clear();
		}

		m_Swapchain = VulkanSwapchain::Create(m_Device, m_Surface, m_Swapchain, config);
	}



}