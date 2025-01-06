#include "deako_pch.h"
#include "VulkanPhysicalDevice.h"

namespace Deako {

	VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice device)
		: m_Device(device)
	{
		vkGetPhysicalDeviceFeatures(m_Device, &m_Features);

		vkGetPhysicalDeviceProperties(m_Device, &m_Properties);

		DK_CORE_INFO("Physical Device: {0}", m_Properties.deviceName);

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &queueFamilyCount, nullptr);

		m_QueueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &queueFamilyCount, m_QueueFamilies.data());
	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{
	}

}