#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

	/// PhysicalDevice encapsulates a VkPhysicalDevice
	class VulkanPhysicalDevice : public Inheritor<Object, VulkanPhysicalDevice>
	{
	public:
		VulkanPhysicalDevice(VkPhysicalDevice device);
		~VulkanPhysicalDevice();

		operator VkPhysicalDevice() const { return m_Device; }

	private:
		VkPhysicalDevice m_Device;

		VkPhysicalDeviceFeatures m_Features;
		VkPhysicalDeviceProperties m_Properties;
		std::vector<VkQueueFamilyProperties> m_QueueFamilies;
	};

	DK_TYPE_NAME(VulkanPhysicalDevice);

}

