#include "deako_pch.h"
#include "VulkanPhysicalDevice.h"

#include "VulkanSurface.h"
#include "VulkanDebugMessenger.h"

namespace Deako {

	VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice device)
		: m_Device(device)
	{
		vkGetPhysicalDeviceFeatures(m_Device, &m_Features);

		vkGetPhysicalDeviceProperties(m_Device, &m_Properties);

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &queueFamilyCount, nullptr);

		m_QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &queueFamilyCount, m_QueueFamilyProperties.data());
	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{
	}

	bool VulkanPhysicalDevice::SupportsDeivceExtensions(const std::vector<const char*>& extensions)
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(m_Device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensionProperties(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_Device, nullptr, &extensionCount, extensionProperties.data());

		std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
		for (const auto& extensionProperty : extensionProperties)
			requiredExtensions.erase(extensionProperty.extensionName);

		return requiredExtensions.empty(); // if empty, extensions are supported
	}

	std::pair<int, int> VulkanPhysicalDevice::GetQueueFamilies(Ref<VulkanSurface> surface)
	{
		uint32_t queueFamilyIndex = 0;
		for (const auto& queueFamily : m_QueueFamilyProperties)
		{
			bool graphicsQueueSupported = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT);

			VkBool32 presentQueueSupported = false;
			VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_Device, queueFamilyIndex, surface->Vk(), &presentQueueSupported));

			if (graphicsQueueSupported && presentQueueSupported)
			{
				m_GraphicsFamily = m_PresentFamily = queueFamilyIndex;
				return { m_GraphicsFamily, m_PresentFamily };
			}

			if (graphicsQueueSupported) m_GraphicsFamily = queueFamilyIndex;
			if (presentQueueSupported) m_PresentFamily = queueFamilyIndex;

			queueFamilyIndex++;
		}

		return { m_GraphicsFamily, m_PresentFamily };
	}

}